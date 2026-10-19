// kaoru/include/llm_bridge.h
#ifndef LLM_BRIDGE_H
#define LLM_BRIDGE_H

#include <stddef.h>
#include "connectome.h"

typedef struct {
    void *model;
    void *ctx;
    void *vocab;
    int   n_threads;
    int   n_ctx;
    int   loaded;
} KaoruLLMBridge;

typedef struct {
    float temperature;
    float top_p;
    float presence_penalty;
    float frequency_penalty;
    int   max_tokens;
    int   seed;
} SamplingParams;

/* Phase A: pure LLM control — no gating, no biasing */
int  kaoru_llm_init(KaoruLLMBridge *b, const char *model_path, int n_threads, int n_ctx);
void kaoru_llm_free(KaoruLLMBridge *b);

/* Map mental state → params (the CORE thing being validated) */
SamplingParams map_mental_state_to_params(const KaoruMentalState *m);

/* Raw generation — deliberately dumb */
int kaoru_generate_raw(KaoruLLMBridge *b,
                       const char *system_prompt,
                       const char *user_prompt,
                       SamplingParams p,
                       char *out, size_t out_len);

#endif