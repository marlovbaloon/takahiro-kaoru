#include "include/connectome.h"
#include <string.h>
#include <math.h>

/* 
 * Initialize structured neural connectome with hierarchical cortical layers
 * and anti-seizure directional routing.
 */
void init_connectome(NeuralConnectome *c) {
    c->neuron_count = 20;     // Expanded micro-scale for layered routing
    c->synapse_count = 30;
    c->integrated_phi = 0.1f;
    c->name_familiarity = 0.0f;
    c->subjective = 0x00;
    
    c->samsara.current_node_stage = 0;
    c->samsara.loop_momentum = 0.0f;
    c->samsara.is_latching = 0;

    // Initialize Cetasika Matrix to Pristine Baseline (Prabhasara state)
    memset(&c->cetasika, 0, sizeof(CetasikaMatrix));
    c->cetasika.equanimity_neutrality = 0.8f; // High baseline calm
    c->cetasika.mindfulness_retention = 0.7f; // Active state monitor
    c->cetasika.wisdom_logic = 0.6f;

    // Initialize layered neurons (Anti-seizure topology: Feedforward preference)
    // Layers: 0 = Sensory, 1 = Cetasika/Limbic, 2 = Association, 3 = Motor
    for (int i = 0; i < c->neuron_count; i++) {
        c->neurons[i].id = i;
        c->neurons[i].cortical_layer = i / 5; // Divide 20 neurons into 4 distinct layers
        c->neurons[i].membrane_potential = 0.0f;
        c->neurons[i].threshold = 1.5f;       // Safe threshold to prevent premature spiking
        c->neurons[i].decay_rate = 0.2f;      // Aggressive natural leak
        c->neurons[i].refractory_ticks = 0;   // No active lockout initially
    }

    // Initialize structured feedforward & controlled recurrent synapses
    int s_idx = 0;
    for (int i = 0; i < c->neuron_count - 1 && s_idx < MAX_SYNAPSES; i++) {
        // Enforce strict directional rule: Prevent backward loops that trigger seizures
        uint8_t pre = i;
        uint8_t post = (i + 1) % c->neuron_count;
        
        // Only allow forward or same-layer stable connections
        if (c->neurons[post].cortical_layer >= c->neurons[pre].cortical_layer) {
            c->synapses[s_idx].pre_neuron_id = pre;
            c->synapses[s_idx].post_neuron_id = post;
            c->synapses[s_idx].weight = 0.45f;
            s_idx++;
        }
    }
    c->synapse_count = s_idx;
}

/*
 * Process external text input and inject into Layer 0 (Sensory Gate).
 */
void feed_stimulus(NeuralConnectome *c, const char *input_text) {
    int len = strlen(input_text);
    if (len == 0) return;

    int is_own_name = (strstr(input_text, "かおる") != NULL);
    float stimulus_energy = 0.0f;

    if (is_own_name) {
        c->name_familiarity += 0.05f;
        if (c->name_familiarity > 1.0f) c->name_familiarity = 1.0f;

        float habituation_factor = 1.0f - (c->name_familiarity * 0.4f);
        stimulus_energy = ((float)len * 0.3f) * habituation_factor;
        
        // Trigger Cetasika reaction for self-recognition
        c->cetasika.conceit_pride += 0.3f;
        c->cetasika.aversion_hostility += 0.2f; // Polite coldness trigger
    } else {
        stimulus_energy = (float)len * 0.25f;
    }

    // Inject energy strictly into Layer 0 sensory neurons
    for (int i = 0; i < c->neuron_count; i++) {
        if (c->neurons[i].cortical_layer == 0) {
            c->neurons[i].membrane_potential += stimulus_energy;
            break;
        }
    }

    // Kickstart Samsara loop cycle upon sensory contact (Phassa -> Vedana)
    c->samsara.current_node_stage = 4; // Node 4: Phassa (Contact)
    c->samsara.loop_momentum += stimulus_energy;
    c->samsara.is_latching = 1;
}

/*
 * Recursive Cetasika Processor: Modulates mental factors based on current neural potentials.
 */
void update_cetasika_matrix(NeuralConnectome *c) {
    float total_potential = 0.0f;
    for (int i = 0; i < c->neuron_count; i++) {
        total_potential += c->neurons[i].membrane_potential;
    }
    float avg_pot = total_potential / (float)c->neuron_count;

    // Recursive feedback: Cetasika adjusts based on neural activity, 
    // and in turn, damping factors regulate neural excitability (Homeostatic loop).
    if (avg_pot > 2.0f) {
        // High arousal -> trigger defense / aversion & logic focus
        c->cetasika.aversion_hostility += 0.05f;
        c->cetasika.wisdom_logic += 0.03f;
        c->cetasika.equanimity_neutrality -= 0.05f;
    } else {
        // Low activity -> pull back toward pristine equanimity (Prabhasara baseline)
        c->cetasika.equanimity_neutrality += 0.02f;
        c->cetasika.aversion_hostility -= 0.03f;
    }

    // Clamp all Cetasika values between 0.0f and 1.0f
    float *ptr = (float*)&c->cetasika;
    for (int i = 0; i < sizeof(CetasikaMatrix) / sizeof(float); i++) {
        if (ptr[i] > 1.0f) ptr[i] = 1.0f;
        if (ptr[i] < 0.0f) ptr[i] = 0.0f;
    }
}

/*
 * Samsara Loop Tracker: Simulates the dependent origination recursive loop stages.
 */
void update_samsara_loop(NeuralConnectome *c) {
    if (!c->samsara.is_latching) return;

    // Advance loop stage recursively based on momentum
    c->samsara.current_node_stage++;
    
    if (c->samsara.current_node_stage >= 12) {
        // Stage 12 reached (Jara-marana / Decay & Reset)
        // Automatically reset back to Prabhasara (Pristine state)
        c->samsara.current_node_stage = 0;
        c->samsara.loop_momentum *= 0.2f; // Decay momentum
        
        if (c->samsara.loop_momentum < 0.1f) {
            c->samsara.is_latching = 0; // Release latch, return to neutral pristine baseline
        }
    }
}

/*
 * Execute one single simulation tick of neural dynamics with Anti-Seizure Safeguards:
 * - Refractory period check (prevents continuous runaway firing)
 * - Layered directional propagation
 * - Recursive Cetasika and Samsara feedback integration
 */
void update_neural_state(NeuralConnectome *c) {
    float active_firing_count = 0.0f;
    float next_potentials[MAX_NEURONS];

    // Step 1: Process firing thresholds, leaky decay, and anti-seizure refractory periods
    for (int i = 0; i < c->neuron_count; i++) {
        next_potentials[i] = c->neurons[i].membrane_potential;

        if (c->neurons[i].refractory_ticks > 0) {
            // Anti-Seizure Lockout: Neuron is resting after an action potential
            c->neurons[i].refractory_ticks--;
            next_potentials[i] -= c->neurons[i].decay_rate * 1.5f;
            if (next_potentials[i] < 0.0f) next_potentials[i] = 0.0f;
            continue;
        }

        if (c->neurons[i].membrane_potential >= c->neurons[i].threshold) {
            active_firing_count += 1.0f;
            // Trigger refractory period (prevent seizure loop)
            c->neurons[i].refractory_ticks = 3; 
            // Post-firing reset
            next_potentials[i] = c->neurons[i].membrane_potential * 0.3f;
        } else {
            // Standard Leaky Integrate-and-Fire decay
            next_potentials[i] -= c->neurons[i].decay_rate;
            if (next_potentials[i] < 0.0f) next_potentials[i] = 0.0f;
        }
    }

    // Step 2: Propagate electrical charge safely across valid directional synapses
    for (int i = 0; i < c->synapse_count; i++) {
        Synapse *s = &c->synapses[i];
        // Inhibitory dampening via equanimity (Prabhasara factor dampens runaway noise)
        float dampening = 1.0f - (c->cetasika.equanimity_neutrality * 0.3f);
        float transferred = c->neurons[s->pre_neuron_id].membrane_potential * s->weight * dampening;
        
        // Prevent overflow into post-synaptic neuron
        if (next_potentials[s->post_neuron_id] + transferred < 5.0f) {
            next_potentials[s->post_neuron_id] += transferred;
        }
    }

    // Step 3: Commit updated potentials with safety clamping
    for (int i = 0; i < c->neuron_count; i++) {
        c->neurons[i].membrane_potential = next_potentials[i];
    }

    // Step 4: Run Recursive Cetasika and Samsara Loop Integration
    update_cetasika_matrix(c);
    update_samsara_loop(c);

    // Step 5: Update Integrated Information Index (Phi)
    c->integrated_phi += (active_firing_count * 0.015f) - (0.02f);
    if (c->integrated_phi < 0.01f) c->integrated_phi = 0.01f;

    // Step 6: Update subjective state flag
    float raw_subjective = (c->name_familiarity * 120.0f) + (c->integrated_phi * 80.0f) + (c->samsara.current_node_stage * 5.0f);
    c->subjective = (raw_subjective > 255.0f) ? 0xFF : (raw_subjective < 0.0f ? 0x00 : (uint8_t)raw_subjective);
}

/*
 * Map internal distributed neural dynamics & Cetasika factors into Kaoru's mental state.
 */
void update_kaoru_mental_state(NeuralConnectome *c, KaoruMentalState *state) {
    // Agency is driven by volition and intent-to-act from Cetasika
    float target_agency = (c->cetasika.volition * 0.4f) + (c->cetasika.intent_to_act * 0.4f) + (c->integrated_phi * 0.2f);
    state->agency_drive = (state->agency_drive * 0.7f) + (target_agency * 0.3f);

    // Relational distance is driven by aversion and self-respect (Polite Coldness)
    float target_distance = 0.4f + (c->cetasika.aversion_hostility * 0.4f) + (c->cetasika.self_respect_dignity * 0.2f);
    if (target_distance > state->relational_distance) {
        state->relational_distance = (state->relational_distance * 0.4f) + (target_distance * 0.6f);
    } else {
        state->relational_distance -= 0.005f;
        if (state->relational_distance < 0.3f) state->relational_distance = 0.3f;
    }

    // Logic focus is driven by wisdom and sustained evaluation
    float target_logic = (c->cetasika.wisdom_logic * 0.5f) + (c->cetasika.sustained_evaluation * 0.5f);
    state->logic_focus = (state->logic_focus * 0.6f) + (target_logic * 0.4f);
}

/*
 * Idle decay: returns the system smoothly back to the Prabhasara (Pristine Equanimity) baseline.
 */
void tick_idle_decay(NeuralConnectome *c) {
    if (c->name_familiarity > 0.0f) c->name_familiarity -= 0.01f;
    if (c->integrated_phi > 0.05f) c->integrated_phi -= 0.005f;

    // Naturally decay active mental factors back toward pristine baseline
    c->cetasika.aversion_hostility -= 0.02f;
    c->cetasika.conceit_pride -= 0.02f;
    c->cetasika.equanimity_neutrality += 0.02f; // Restore calm baseline

    if (c->cetasika.equanimity_neutrality > 1.0f) c->cetasika.equanimity_neutrality = 1.0f;

    for (int i = 0; i < c->neuron_count; i++) {
        c->neurons[i].membrane_potential -= c->neurons[i].decay_rate;
        if (c->neurons[i].membrane_potential < 0.0f) {
            c->neurons[i].membrane_potential = 0.0f;
        }
        if (c->neurons[i].refractory_ticks > 0) {
            c->neurons[i].refractory_ticks--;
        }
    }
}
