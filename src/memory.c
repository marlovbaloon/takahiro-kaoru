// kaoru/src/memory.c
#include "connectome.h"
#include <string.h>
#include <math.h>
#include <time.h>

static float cetasika_distance(const CetasikaMatrix *a, const CetasikaMatrix *b) {
    const float *pa = (const float *)a;
    const float *pb = (const float *)b;
    size_t n = sizeof(CetasikaMatrix) / sizeof(float);
    float acc = 0.0f;
    for (size_t i = 0; i < n; i++) {
        float d = pa[i] - pb[i];
        acc += d * d;
    }
    return sqrtf(acc / (float)n);
}

float resonance_score(const NeuralConnectome *c, const EpisodicMemoryNode *m) {
    const float W_PHI      = 0.30f;
    const float W_CETASIKA = 0.35f;
    const float W_SAMSARA  = 0.15f;
    const float W_SALIENCE = 0.20f;

    float phi_sim = 1.0f - fabsf(c->integrated_phi - m->phi_at_encoding);
    if (phi_sim < 0.0f) phi_sim = 0.0f;
    if (phi_sim > 1.0f) phi_sim = 1.0f;

    float cet_sim = 1.0f - cetasika_distance(&c->cetasika, &m->cetasika_snapshot);
    if (cet_sim < 0.0f) cet_sim = 0.0f;
    if (cet_sim > 1.0f) cet_sim = 1.0f;

    float samsara_match =
        (c->samsara.current_node_stage == m->samsara_stage_at_encoding) ? 1.0f : 0.0f;

    int64_t now = (int64_t)time(NULL);
    int64_t age = now - m->created_at;
    if (age < 0) age = 0;
    float recency  = 1.0f / (1.0f + (float)age / 86400.0f);
    float sal_term = m->salience * (0.5f + 0.5f * recency);

    float r = W_PHI * phi_sim
            + W_CETASIKA * cet_sim
            + W_SAMSARA * samsara_match
            + W_SALIENCE * sal_term;

    if (r < 0.0f) r = 0.0f;
    if (r > 1.0f) r = 1.0f;
    return r;
}

int find_resonant_memory(const NeuralConnectome *c, int *out, int max_results) {
    if (!out || max_results <= 0 || c->episodic_count == 0) return 0;

    const float THRESHOLD = 0.55f;
    typedef struct { float s; int idx; } Scored;
    Scored list[MAX_EPISODIC_MEMORIES];
    int n = 0;

    for (int i = 0; i < c->episodic_count; i++) {
        float s = resonance_score(c, &c->memories[i]);
        if (s < THRESHOLD) continue;
        /* insertion-sort into descending list */
        int j = n;
        while (j > 0 && list[j - 1].s < s) {
            list[j] = list[j - 1];
            j--;
        }
        if (j < MAX_EPISODIC_MEMORIES) {
            list[j].s = s;
            list[j].idx = i;
            if (n < MAX_EPISODIC_MEMORIES) n++;
        }
    }

    int out_n = (n < max_results) ? n : max_results;
    for (int i = 0; i < out_n; i++) out[i] = list[i].idx;
    return out_n;
}

void encode_episodic_memory(NeuralConnectome *c, const char *tag) {
    float arousal = c->cetasika.aversion_hostility
                  + c->cetasika.greed_attachment
                  + c->cetasika.restlessness;
    if (arousal > 1.0f) arousal = 1.0f;

    float salience = 0.4f * c->integrated_phi + 0.6f * arousal;
    if (salience < 0.05f) return;
    if (salience > 1.0f) salience = 1.0f;

    if (c->episodic_count >= MAX_EPISODIC_MEMORIES) consolidate_memories(c);
    if (c->episodic_count >= MAX_EPISODIC_MEMORIES) return;  /* still full after consolidate */

    EpisodicMemoryNode *m = &c->memories[c->episodic_count++];
    memset(m, 0, sizeof(*m));
    m->memory_id          = c->next_memory_id++;
    m->created_at         = (int64_t)time(NULL);
    m->salience           = salience;
    m->phi_at_encoding    = c->integrated_phi;
    m->samsara_stage_at_encoding = c->samsara.current_node_stage;
    memcpy(&m->cetasika_snapshot, &c->cetasika, sizeof(CetasikaMatrix));

    if (tag) {
        size_t n = strlen(tag);
        if (n >= MAX_MEMORY_TAG_LEN) n = MAX_MEMORY_TAG_LEN - 1;
        memcpy(m->tag, tag, n);
        m->tag[n] = '\0';
    }
}

void consolidate_memories(NeuralConnectome *c) {
    const float PRUNE = 0.10f;
    const float DECAY = 0.95f;

    for (int i = 0; i < c->episodic_count; i++) {
        c->memories[i].salience *= DECAY;
    }
    int write = 0;
    for (int i = 0; i < c->episodic_count; i++) {
        if (c->memories[i].salience >= PRUNE) {
            if (write != i) c->memories[write] = c->memories[i];
            write++;
        }
    }
    c->episodic_count = (uint8_t)write;
}