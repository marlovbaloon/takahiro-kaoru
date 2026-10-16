#include "include/connectome.h"
#include <string.h>
#include <math.h>

/* 
 * Initialize the micro-connectome network with baseline parameters.
 * Sets up initial neuron membrane potentials, firing thresholds, 
 * decay rates, and default synaptic weights.
 */
void init_connectome(FruitFlyConnectome *c) {
    c->neuron_count = 10;
    c->synapse_count = 12;
    c->integrated_phi = 0.1f;
    c->name_familiarity = 0.0f;
    c->subjective = 0x00;
    
    // Initialize individual neurons with leaky integrate-and-fire properties
    for (int i = 0; i < c->neuron_count; i++) {
        c->neurons[i].id = i;
        c->neurons[i].membrane_potential = 0.0f;
        c->neurons[i].threshold = 1.2f;
        c->neurons[i].decay_rate = 0.15f;
    }

    // Initialize recurrent synaptic connections between neurons
    for (int i = 0; i < c->synapse_count; i++) {
        c->synapses[i].pre_neuron_id = i % c->neuron_count;
        c->synapses[i].post_neuron_id = (i + 1) % c->neuron_count;
        c->synapses[i].weight = 0.6f;
    }
}

/*
 * Process external text input, simulate sensory reception, 
 * and apply habituation if the input matches specific triggers (e.g., own name).
 */
void feed_stimulus(FruitFlyConnectome *c, const char *input_text) {
    int len = strlen(input_text);
    if (len == 0) return;

    // Check if the sensory input contains self-identity recognition trigger ("かおる")
    int is_own_name = (strstr(input_text, "かおる") != NULL);
    float stimulus_energy = 0.0f;

    if (is_own_name) {
        // Increase familiarity plasticity with repetition
        c->name_familiarity += 0.05f;
        if (c->name_familiarity > 1.0f) {
            c->name_familiarity = 1.0f;
        }

        // Habituation / Desensitization factor: repeated stimuli yield less excitement (เลี่ยน)
        float habituation_factor = 1.0f - (c->name_familiarity * 0.5f);
        stimulus_energy = ((float)len * 0.35f) * habituation_factor;
    } else {
        // General environmental stimulus without habituation dampening
        stimulus_energy = (float)len * 0.35f;
    }

    // Inject stimulus energy into the primary sensory neuron (Neuron 0)
    c->neurons[0].membrane_potential += stimulus_energy;
    
    // Clamp maximum membrane potential to prevent overflow
    if (c->neurons[0].membrane_potential > 5.0f) {
        c->neurons[0].membrane_potential = 5.0f;
    }
}

/*
 * Execute one single simulation tick of neural dynamics:
 * handles firing thresholds, potential decay, synaptic signal propagation, 
 * and continuous computation of integrated information (Phi) and subjective states.
 */
void update_neural_state(FruitFlyConnectome *c) {
    float active_firing_count = 0.0f;
    float next_potentials[MAX_NEURONS];

    // Step 1: Process firing threshold and natural potential decay for each neuron
    for (int i = 0; i < c->neuron_count; i++) {
        if (c->neurons[i].membrane_potential >= c->neurons[i].threshold) {
            active_firing_count += 1.0f;
            // Post-firing reset / refractory state deduction
            next_potentials[i] = c->neurons[i].membrane_potential * 0.5f;
        } else {
            // Leaky integrate-and-fire decay over time
            next_potentials[i] = c->neurons[i].membrane_potential - c->neurons[i].decay_rate;
            if (next_potentials[i] < 0.0f) {
                next_potentials[i] = 0.0f;
            }
        }
    }

    // Step 2: Propagate electrical charge across synaptic connections
    for (int i = 0; i < c->synapse_count; i++) {
        Synapse *s = &c->synapses[i];
        float transferred = c->neurons[s->pre_neuron_id].membrane_potential * s->weight;
        next_potentials[s->post_neuron_id] += transferred;
    }

    // Step 3: Update membrane potentials with safety clamping
    for (int i = 0; i < c->neuron_count; i++) {
        c->neurons[i].membrane_potential = next_potentials[i];
        if (c->neurons[i].membrane_potential > 5.0f) {
            c->neurons[i].membrane_potential = 5.0f;
        }
    }

    // Step 4: Update Integrated Information Index (Phi)
    c->integrated_phi += (active_firing_count * 0.02f) - (c->neurons[0].decay_rate * 0.05f);
    if (c->integrated_phi < 0.01f) {
        c->integrated_phi = 0.01f;
    }

    // Step 5: Continuous subjective state mapping (Eliminating rigid if-else blocks)
    // Combines familiarity and integrated info into a smooth 0x00 - 0xFF emotional gradient byte
    float raw_subjective_score = (c->name_familiarity * 150.0f) + (c->integrated_phi * 100.0f);
    
    if (raw_subjective_score > 255.0f) {
        c->subjective = 0xFF;
    } else if (raw_subjective_score < 0.0f) {
        c->subjective = 0x00;
    } else {
        c->subjective = (uint8_t)raw_subjective_score;
    }
}
/*
 * Map internal distributed neural dynamics into Kaoru's high-level mental state vector.
 * Utilizes multi-neuron activation patterns, variance, and persistent memory (familiarity/phi)
 * to drive agency, relational distance (with sticky defense decay), and logic focus.
 */
void update_kaoru_mental_state(FruitFlyConnectome *c, KaoruMentalState *state) {
    float total_potential = 0.0f;
    float active_sum = 0.0f;
    
    // Step 1: Analyze distributed membrane potentials across the network
    for (int i = 0; i < c->neuron_count; i++) {
        total_potential += c->neurons[i].membrane_potential;
        if (c->neurons[i].membrane_potential >= c->neurons[i].threshold * 0.8f) {
            active_sum += 1.0f;
        }
    }
    
    float mean_potential = total_potential / (float)c->neuron_count;

    // Step 2: Drive Agency (Goal focus / Drive)
    // Driven by overall network activation energy and integrated phi
    float target_agency = (mean_potential * 0.2f) + (c->integrated_phi * 0.3f);
    if (target_agency > 1.0f) target_agency = 1.0f;
    if (target_agency < 0.0f) target_agency = 0.0f;
    
    // Smooth transition (momentum) for agency
    state->agency_drive = (state->agency_drive * 0.7f) + (target_agency * 0.3f);

    // Step 3: Drive Relational Distance (Sticky defense mode / Polite Coldness)
    // Triggered heavily when name familiarity or repetition is high (habituation/annoyance)
    float target_distance = 0.3f + (c->name_familiarity * 0.6f);
    if (target_distance > 1.0f) target_distance = 1.0f;

    // Sticky rule: relational_distance drops MUCH slower than it rises to maintain personality tone
    if (target_distance > state->relational_distance) {
        // Fast rise (quick to put up defenses when triggered)
        state->relational_distance = (state->relational_distance * 0.4f) + (target_distance * 0.6f);
    } else {
        // Extremely slow decay (hard to drop guard once set)
        state->relational_distance = state->relational_distance - 0.005f;
        if (state->relational_distance < 0.2f) {
            state->relational_distance = 0.2f; // Minimum baseline distance
        }
    }

    // Step 4: Drive Logic Focus (Calculation / Structural analysis mode)
    // Inversely modulated by active emotional firing or high idle decay
    float target_logic = 0.5f + (mean_potential * -0.1f) + ((float)active_sum / (float)c->neuron_count * 0.4f);
    if (target_logic > 1.0f) target_logic = 1.0f;
    if (target_logic < 0.0f) target_logic = 0.0f;
    
    state->logic_focus = (state->logic_focus * 0.6f) + (target_logic * 0.4f);
}
/*
 * Simulate idle state decay during periods of user silence.
 * Allows residual feelings, familiarity, and neural potential to fade naturally over time.
 */
void tick_idle_decay(FruitFlyConnectome *c) {
    // Forgiving memory decay (forgetting familiarity over time without input)
    if (c->name_familiarity > 0.0f) {
        c->name_familiarity -= 0.01f;
        if (c->name_familiarity < 0.0f) {
            c->name_familiarity = 0.0f;
        }
    }

    // Decay integrated Phi back to baseline
    c->integrated_phi -= 0.005f;
    if (c->integrated_phi < 0.01f) {
        c->integrated_phi = 0.01f;
    }

    // Residual potential fading across all neurons during silence (emotional hangover fading)
    for (int i = 0; i < c->neuron_count; i++) {
        c->neurons[i].membrane_potential -= (c->neurons[i].decay_rate * 0.5f);
        if (c->neurons[i].membrane_potential < 0.0f) {
            c->neurons[i].membrane_potential = 0.0f;
        }
    }

    // Continuously update subjective state byte during idle decay
    float raw_subjective_score = (c->name_familiarity * 150.0f) + (c->integrated_phi * 100.0f);
    
    if (raw_subjective_score > 255.0f) {
        c->subjective = 0xFF;
    } else if (raw_subjective_score < 0.0f) {
        c->subjective = 0x00;
    } else {
        c->subjective = (uint8_t)raw_subjective_score;
    }
}
