// kaoru/src/feedback.c
#include "feedback.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* ============ Config ============ */
void feedback_config_default(FeedbackConfig *cfg) {
    cfg->self_stimulus_weight        = 0.40f;  /* 40% of user weight */
    cfg->echo_suppression_threshold  = 0.65f;  /* skip if >65% similar to input */
    cfg->metacognitive_gain          = 0.30f;
    cfg->reflection_ticks            = 2;
    cfg->memory_encoding_threshold   = 0.35f;
    cfg->min_output_len              = 5.0f;
}

/* ============ Helpers ============ */

/* Char-frequency similarity (fast, good enough for echo suppression) */
static float text_similarity(const char *a, const char *b) {
    if (!a || !b || !*a || !*b) return 0.0f;
    int fa[256] = {0}, fb[256] = {0};
    int la = 0, lb = 0;
    for (const unsigned char *p = (const unsigned char *)a; *p; p++) { fa[*p]++; la++; }
    for (const unsigned char *p = (const unsigned char *)b; *p; p++) { fb[*p]++; lb++; }
    int common = 0;
    for (int i = 0; i < 256; i++) common += (fa[i] < fb[i]) ? fa[i] : fb[i];
    return (la + lb == 0) ? 0.0f : (2.0f * (float)common / (float)(la + lb));
}

/* Copy output into tag, replacing control chars but keeping UTF-8 bytes */
static void sanitize_tag(char *dst, size_t dst_len, const char *src, size_t max_src) {
    size_t i = 0;
    for (; i < max_src && src[i] && i < dst_len - 1; i++) {
        unsigned char ch = (unsigned char)src[i];
        if (ch < 32) ch = ' ';
        dst[i] = (char)ch;
    }
    dst[i] = '\0';
}

/* Clamp all Cetasika values to [0,1] */
static void clamp_cetasika(NeuralConnectome *c) {
    float *ptr = (float *)&c->cetasika;
    size_t n = sizeof(CetasikaMatrix) / sizeof(float);
    for (size_t i = 0; i < n; i++) {
        if (ptr[i] < 0.0f) ptr[i] = 0.0f;
        if (ptr[i] > 1.0f) ptr[i] = 1.0f;
    }
}

/* Metacognitive evaluation of own output.
 * Compares what was said vs current internal state → produces dissonance or harmony.
 */
static void self_evaluate(NeuralConnectome *c, const char *output, float gain) {
    if (!output || !*output) return;

    /* --- 1. Aggression markers vs current aversion --- */
    const char *agg_markers[] = {
        "hate", "stupid", "idiot", "fool", "shut up", "angry",
        "โง่", "เกลียด", "หุบปาก", "โมโห", NULL
    };
    int aggressive = 0;
    for (int i = 0; agg_markers[i]; i++) {
        if (strstr(output, agg_markers[i])) { aggressive = 1; break; }
    }
    if (aggressive) {
        /* Dissonance: said aggressive things but wasn't that hostile */
        float dissonance = (1.0f - c->cetasika.aversion_hostility) * gain;
        c->cetasika.regret_remorse   += dissonance * 0.5f;
        c->cetasika.prudent_dread    += dissonance * 0.3f;
        c->cetasika.aversion_hostility += dissonance * 0.2f;
    }

    /* --- 2. Uncertainty markers --- */
    const char *unc_markers[] = {
        "maybe", "perhaps", "not sure", "unclear", "unsure",
        "อาจจะ", "ไม่แน่ใจ", "?", NULL
    };
    int uncertain = 0;
    for (int i = 0; unc_markers[i]; i++) {
        if (strstr(output, unc_markers[i])) { uncertain = 1; break; }
    }
    if (uncertain) {
        c->cetasika.skepticism_doubt += gain * 0.10f;
        c->cetasika.decision         -= gain * 0.05f;
    } else {
        /* Confident speech → reinforce conviction */
        c->cetasika.decision         += gain * 0.05f;
        c->cetasika.trust_conviction += gain * 0.03f;
    }

    /* --- 3. Length-based cognitive load --- */
    float out_len = (float)strlen(output);
    if (out_len > 200.0f) {
        c->cetasika.sustained_evaluation += gain * 0.10f;
        c->cetasika.sloth                += gain * 0.05f;  /* tired */
    } else if (out_len < 30.0f) {
        c->cetasika.initial_thought      += gain * 0.05f;
        c->cetasika.concentration        -= gain * 0.03f;
    }

    /* --- 4. Reflect on self-consistency: if Kaoru speaks calmly
     *         but state is chaotic → increase mindfulness --- */
    float internal_noise = c->cetasika.restlessness
                         + c->cetasika.skepticism_doubt
                         + c->cetasika.aversion_hostility;
    if (internal_noise > 1.5f && out_len > 80.0f) {
        /* Managed to speak coherently despite noise → strengthen equanimity */
        c->cetasika.equanimity_neutrality += gain * 0.08f;
        c->cetasika.mindfulness_retention += gain * 0.06f;
    }

    clamp_cetasika(c);
}

/* ============ Main entry point ============ */
void apply_self_feedback(NeuralConnectome *c,
                         KaoruMentalState *mental,
                         const char *user_input,
                         const char *kaoru_output,
                         const FeedbackConfig *cfg) {
    if (!c || !kaoru_output || !*kaoru_output) return;
    if (!cfg) return;

    float out_len = (float)strlen(kaoru_output);
    if (out_len < cfg->min_output_len) return;   /* too trivial to feed back */

    /* ---- 1. Echo suppression: don't reinforce mirroring user ---- */
    if (user_input && *user_input) {
        float sim = text_similarity(user_input, kaoru_output);
        if (sim > cfg->echo_suppression_threshold) {
            /* Said essentially the same thing as user → skip to avoid echo chamber */
            return;
        }
    }

    /* ---- 2. Attenuate by length (short output = less state impact) ---- */
    float length_factor = out_len / 200.0f;
    if (length_factor > 1.0f) length_factor = 1.0f;
    float effective_weight = cfg->self_stimulus_weight * length_factor;

    /* ---- 3. Inject self-speech as stimulus ---- */
    feed_stimulus_sourced(c, kaoru_output, effective_weight, STIM_SOURCE_SELF);

    /* ---- 4. Metacognitive self-evaluation ---- */
    self_evaluate(c, kaoru_output, cfg->metacognitive_gain);

    /* ---- 5. Reflection ticks: let perturbation propagate ---- */
    for (int i = 0; i < cfg->reflection_ticks; i++) {
        update_neural_state(c);
        update_wisdom_module(c);
        update_synaptic_plasticity(c);
    }
    apply_wisdom_filtering(c);

    /* ---- 6. Update mental state to reflect post-feedback reality ---- */
    if (mental) {
        update_kaoru_mental_state(c, mental);
    }

    /* ---- 7. Encode self-speech as memory if salient ---- */
    if (c->integrated_phi > cfg->memory_encoding_threshold) {
        char tag[MAX_MEMORY_TAG_LEN];
        char safe[MAX_MEMORY_TAG_LEN];
        sanitize_tag(safe, sizeof(safe), kaoru_output, 60);
        snprintf(tag, sizeof(tag), "[self] %s", safe);
        encode_episodic_memory(c, tag);
    }
}