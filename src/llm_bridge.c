// kaoru/src/llm_bridge.c
#include "llm_bridge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "llama.h"

int kaoru_llm_init(KaoruLLMBridge *b, const char *model_path, int n_threads, int n_ctx) {
    memset(b, 0, sizeof(*b));
    llama_backend_init();

    struct llama_model_params mp = llama_model_default_params();
    mp.n_gpu_layers = 0;
    b->model = llama_model_load_from_file(model_path, mp);
    if (!b->model) return -1;

    struct llama_context_params cp = llama_context_default_params();
    cp.n_ctx     = (uint32_t)n_ctx;
    cp.n_threads = n_threads;
    cp.n_batch   = 512;
    b->ctx = llama_init_from_model(b->model, cp);
    if (!b->ctx) { llama_model_free(b->model); return -1; }

    b->vocab     = (void *)llama_model_get_vocab(b->model);
    b->n_threads = n_threads;
    b->n_ctx     = n_ctx;
    b->loaded    = 1;
    return 0;
}

void kaoru_llm_free(KaoruLLMBridge *b) {
    if (!b) return;
    if (b->ctx)   llama_free(b->ctx);
    if (b->model) llama_model_free(b->model);
    llama_backend_free();
    memset(b, 0, sizeof(*b));
}

SamplingParams map_mental_state_to_params(const KaoruMentalState *m) {
    SamplingParams p;
    p.temperature       = 0.9f - (m->logic_focus * 0.6f);
    p.top_p             = 0.95f - (m->logic_focus * 0.25f);
    p.presence_penalty  = 0.1f + (m->relational_distance * 0.4f);
    p.frequency_penalty = 0.1f + (m->relational_distance * 0.3f);
    p.max_tokens        = (int)(128.0f + m->agency_drive * 384.0f);
    p.seed              = -1;
    if (p.temperature < 0.05f) p.temperature = 0.05f;
    if (p.top_p < 0.10f)       p.top_p = 0.10f;
    if (p.max_tokens < 32)     p.max_tokens = 32;
    return p;
}

static int build_chatml(const char *sys, const char *usr, char *out, size_t n) {
    return snprintf(out, n,
        "<|im_start|>system\n%s<|im_end|>\n"
        "<|im_start|>user\n%s<|im_end|>\n"
        "<|im_start|>assistant\n",
        sys ? sys : "", usr ? usr : "");
}

int kaoru_generate_raw(KaoruLLMBridge *b, const char *system_prompt,
                       const char *user_prompt, SamplingParams p,
                       char *out, size_t out_len) {
    if (!b || !b->loaded || !out || out_len == 0) return -1;
    if (p.seed < 0) p.seed = (int)(time(NULL) & 0x7FFFFFFF);

    const struct llama_vocab *vocab = (const struct llama_vocab *)b->vocab;

    char prompt[4096];
    int plen = build_chatml(system_prompt, user_prompt, prompt, sizeof(prompt));
    if (plen <= 0 || plen >= (int)sizeof(prompt)) return -1;

    int32_t prompt_tokens[2048];
    int n_prompt = llama_tokenize(vocab, prompt, plen, prompt_tokens, 2048, true, true);
    if (n_prompt <= 0) return -1;

    struct llama_sampler_chain_params sp = llama_sampler_chain_default_params();
    struct llama_sampler *chain = llama_sampler_chain_init(sp);
    llama_sampler_chain_add(chain, llama_sampler_init_penalties(
        64, p.presence_penalty, p.frequency_penalty, 0.0f));
    llama_sampler_chain_add(chain, llama_sampler_init_top_p(p.top_p, 1));
    llama_sampler_chain_add(chain, llama_sampler_init_temp(p.temperature));
    llama_sampler_chain_add(chain, llama_sampler_init_dist((uint32_t)p.seed));

    llama_memory_clear(llama_get_memory(b->ctx), true);
    struct llama_batch batch = llama_batch_get_one(prompt_tokens, n_prompt);
    if (llama_decode(b->ctx, batch) != 0) { llama_sampler_free(chain); return -1; }

    size_t written = 0;
    int n_gen = 0;
    char piece[256];

    while (n_gen < p.max_tokens && written + 1 < out_len) {
        int32_t tok = llama_sampler_sample(chain, b->ctx, -1);
        llama_sampler_accept(chain, tok);
        if (llama_vocab_is_eog(vocab, tok)) break;

        int pl = llama_token_to_piece(vocab, tok, piece, sizeof(piece), 0, true);
        if (pl > 0) {
            if (written + pl >= out_len) pl = (int)(out_len - written - 1);
            memcpy(out + written, piece, pl);
            written += pl;
        }
        struct llama_batch nb = llama_batch_get_one(&tok, 1);
        if (llama_decode(b->ctx, nb) != 0) break;
        n_gen++;
    }
    out[written] = '\0';
    llama_sampler_free(chain);
    return (int)written;
}