// kaoru/include/feedback.h
#ifndef FEEDBACK_H
#define FEEDBACK_H

#include "connectome.h"

typedef struct {
    float self_stimulus_weight;       /* 0.0-1.0, attenuation for self-speech */
    float echo_suppression_threshold; /* 0.0-1.0, above = skip feedback */
    float metacognitive_gain;         /* how strongly self-eval affects Cetasika */
    int   reflection_ticks;           /* extra neural ticks to propagate */
    float memory_encoding_threshold;  /* Φ threshold to encode as memory */
    float min_output_len;             /* ignore trivially short responses */
} FeedbackConfig;

void feedback_config_default(FeedbackConfig *cfg);

/*
 * Apply self-feedback: Kaoru listens to its own output and updates substrate.
 * This is the "4th intersection" — closes the causal loop.
 */
void apply_self_feedback(NeuralConnectome *c, KaoruMentalState *mental, const char *user_input,  const char *kaoru_output, const FeedbackConfig *cfg);

#endif