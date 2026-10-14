#include "include/connectome.h"
#include <string.h>
#include <math.h>
 
void init_connectome(FruitFlyConnectome *c) {
    c->neuron_count = 10;
    c->synapse_count = 12;
    c->integrated_phi = 0.1f;
    c->name_familiarity = 0.0f;
    c->subjective = 0x00;
    
    for (int i = 0; i < c->neuron_count; i++) {
        c->neurons[i].id = i;
        c->neurons[i].membrane_potential = 0.0f;
        c->neurons[i].threshold = 1.2f;
        c->neurons[i].decay_rate = 0.15f;
    }

    for (int i = 0; i < c->synapse_count; i++) {
        c->synapses[i].pre_neuron_id = i % c->neuron_count;
        c->synapses[i].post_neuron_id = (i + 1) % c->neuron_count;
        c->synapses[i].weight = 0.6f;
    }
}

void feed_stimulus(FruitFlyConnectome *c, const char *input_text) {
    int len = strlen(input_text);
    if (len == 0) return;

    int is_own_name = (strstr(input_text, "かおる") != NULL);
    float stimulus_energy = 0.0f;

    if (is_own_name) {
        c->name_familiarity += 0.05f;
        if (c->name_familiarity > 1.0f) {
            c->name_familiarity = 1.0f;
        }

        float habituation_factor = 1.0f - (c->name_familiarity * 0.5f);
        stimulus_energy = ((float)len * 0.35f) * habituation_factor;
    } else {
        stimulus_energy = (float)len * 0.35f;
    }

    c->neurons[0].membrane_potential += stimulus_energy;
    
    if (c->neurons[0].membrane_potential > 5.0f) {
        c->neurons[0].membrane_potential = 5.0f;
    }
}

void update_neural_state(FruitFlyConnectome *c) {
    float active_firing_count = 0.0f;
    float next_potentials[MAX_NEURONS];

    for (int i = 0; i < c->neuron_count; i++) {
        if (c->neurons[i].membrane_potential >= c->neurons[i].threshold) {
            active_firing_count += 1.0f;
            next_potentials[i] = c->neurons[i].membrane_potential * 0.5f;
        } else {
            next_potentials[i] = c->neurons[i].membrane_potential - c->neurons[i].decay_rate;
            if (next_potentials[i] < 0.0f) {
                next_potentials[i] = 0.0f;
            }
        }
    }

    for (int i = 0; i < c->synapse_count; i++) {
        Synapse *s = &c->synapses[i];
        float transferred = c->neurons[s->pre_neuron_id].membrane_potential * s->weight;
        next_potentials[s->post_neuron_id] += transferred;
    }

    for (int i = 0; i < c->neuron_count; i++) {
        c->neurons[i].membrane_potential = next_potentials[i];
        if (c->neurons[i].membrane_potential > 5.0f) {
            c->neurons[i].membrane_potential = 5.0f;
        }
    }

    c->integrated_phi += (active_firing_count * 0.02f) - (c->neurons[0].decay_rate * 0.05f);
    if (c->integrated_phi < 0.01f) {
        c->integrated_phi = 0.01f;
    }
    float raw_subjective_score = (c->name_familiarity * 150.0f) + (c->integrated_phi * 100.0f);
    
    if (raw_subjective_score > 255.0f) {
        c->subjective = 0xFF;
    } else if (raw_subjective_score < 0.0f) {
        c->subjective = 0x00;
    } else {
        c->subjective = (uint8_t)raw_subjective_score;
    }
}
// Idle / Residual Decay
void tick_idle_decay(FruitFlyConnectome *c) {
    // Forgiving/Fading memory
    if (c->name_familiarity > 0.0f) {
        c->name_familiarity -= 0.01f;
        if (c->name_familiarity < 0.0f) {
            c->name_familiarity = 0.0f;
        }
    }
    c->integrated_phi -= 0.005f;
    if (c->integrated_phi < 0.01f) {
        c->integrated_phi = 0.01f;
    }

    // Residual fading 
    for (int i = 0; i < c->neuron_count; i++) {
        c->neurons[i].membrane_potential -= (c->neurons[i].decay_rate * 0.5f);
        if (c->neurons[i].membrane_potential < 0.0f) {
            c->neurons[i].membrane_potential = 0.0f;
        }
    }

    // Subjective state 
    if (c->name_familiarity >= 0.8f && c->integrated_phi > 0.3f) {
        c->subjective = 0xFF;
    } else if (c->name_familiarity >= 0.4f) {
        c->subjective = 0x7F;
    } else if (c->name_familiarity > 0.0f) {
        c->subjective = 0x1F;
    } else {
        c->subjective = 0x00;
    }
}
