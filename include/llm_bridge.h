// kaoru/include/llm_bridge.h
#ifndef LLM_BRIDGE_H
#define LLM_BRIDGE_H

#include <stddef.h>
#include "connectome.h"

typedef struct {
    char *server_url;   /* เช่น "http://localhost:8080" */
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

int  kaoru_llm_init(KaoruLLMBridge *b, const char *server_url, int n_ctx);
void kaoru_llm_free(KaoruLLMBridge *b);

SamplingParams map_mental_state_to_params(const KaoruMentalState *m);

int kaoru_generate_raw(KaoruLLMBridge *b,
                       const char *system_prompt,
                       const char *user_prompt,
                       SamplingParams p,
                       char *out, size_t out_len);

#endif