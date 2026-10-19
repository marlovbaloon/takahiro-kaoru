// kaoru/src/wisdom.c
#include "connectome.h"
#include <string.h>
#include <math.h>

/*
 * Wisdom Module (Paññā / Sapientia / Sophia)
 * Implements non-delusion (Amoha), penetrative causal analysis,
 * and rational filtering to maintain Kaoru's cold, logical baseline.
 */

/*
 * Evaluates incoming text payloads for logical validity, structural coherence,
 * and causal depth, adjusting the Paññā (wisdom) vector accordingly.
 */
void evaluate_causal_logic(NeuralConnectome *c, const char *input_payload) {
    if (!input_payload) return;

    // Check for logical keywords, structural queries, or utility markers
    int has_logical_structure = (strstr(input_payload, "why") != NULL || 
                                 strstr(input_payload, "how") != NULL || 
                                 strstr(input_payload, "logic") != NULL ||
                                 strstr(input_payload, "cost") != NULL ||
                                 strstr(input_payload, "value") != NULL);

    if (has_logical_structure) {
        // High analytical input reinforces Paññā factor
        c->cetasika.wisdom_logic += 0.08f;
    } else {
        // Trivial or irrational noise slightly taxes speculative focus
        c->cetasika.wisdom_logic -= 0.02f;
    }

    // Clamp structural bounds [0.0f, 1.0f]
    if (c->cetasika.wisdom_logic > 1.00f) c->cetasika.wisdom_logic = 1.00f;
    if (c->cetasika.wisdom_logic < 0.15f) c->cetasika.wisdom_logic = 0.15f;  // was 0.0f
}

/*
 * Active Wisdom Update: Amoha actively suppresses Moha (Delusion) 
 * and dampens unwholesome emotional reactivity (Aversion / Greed).
 */
void update_wisdom_module(NeuralConnectome *c) {
    float current_wisdom = c->cetasika.wisdom_logic;

    if (current_wisdom > 0.4f) {
        // Suppress delusion (Moha) proportionally to active wisdom
        c->cetasika.delusion -= current_wisdom * 0.12f;
        if (c->cetasika.delusion < 0.0f) c->cetasika.delusion = 0.0f;

        // Dampen cold irritation (Dosa) and reckless craving (Lobha) via penetrative clarity
        c->cetasika.aversion_hostility *= (1.0f - (current_wisdom * 0.15f));
        c->cetasika.greed_attachment *= (1.0f - (current_wisdom * 0.10f));
    }

    // Compute target rational baseline driven by sustained evaluation and integrated phi
    float target_wisdom = 0.45f + (c->cetasika.sustained_evaluation * 0.3f) + (c->integrated_phi * 0.25f);

    // Exponential moving average smoothing for smooth cognitive state transitions
    c->cetasika.wisdom_logic = (current_wisdom * 0.75f) + (target_wisdom * 0.25f);

    if (c->cetasika.wisdom_logic > 1.0f) c->cetasika.wisdom_logic = 1.0f;
    if (c->cetasika.wisdom_logic < 0.15f) c->cetasika.wisdom_logic = 0.15f; // Hard rational floor
}

/*
 * Rational Pruning Filter: High wisdom enforces structural efficiency 
 * by zeroing out irrational sub-threshold neural noise (preventing emotional drift).
 */
void apply_wisdom_filtering(NeuralConnectome *c) {
    float pruning_threshold = 0.12f * c->cetasika.wisdom_logic;

    for (int i = 0; i < c->neuron_count; i++) {
        if (c->neurons[i].membrane_potential < pruning_threshold) {
            // Prune low-level emotional static, preserving only high-significance signals
            c->neurons[i].membrane_potential = 0.0f;
        }
    }
}
