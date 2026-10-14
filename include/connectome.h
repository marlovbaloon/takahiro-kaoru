// include/connectome.h
#ifndef CONNECTOME_H
#define CONNECTOME_H

#include <stdint.h>

#define MAX_NEURONS 64      // Micro-unit simulation scale for mobile/MCU deployment
#define MAX_SYNAPSES 256

typedef enum {
    STIM_AUDITORY_NAME = 0, // Active identity / familiarity recognition
    STIM_AUDITORY_GENERAL,  // General environmental or speech sound
    STIM_TACTILE_TOUCH,     // Collision / physical touch in VR
    STIM_ENVIRONMENTAL      // Background ambient stimulus
} StimulusType;             // Fixed missing semicolon here!

typedef struct {
    StimulusType type;
    float intensity;        // Signal magnitude (0.0f - 5.0f)
    const char *payload;    // Text or data mapping payload
} Stimulus;
 
// Node representation structure for an individual neuron (Neuron Node)
typedef struct {
    uint8_t id;
    float membrane_potential;   // Membrane potential value (Signal Accumulation)
    float threshold;            // Action potential activation threshold
    float decay_rate;           // Temporal electric charge decay rate
} Neuron;

// Synaptic connection representation structure (Synapse Edge - Recurrent Loop)
typedef struct {
    uint8_t pre_neuron_id;      // Source neuron
    uint8_t post_neuron_id;     // Target neuron
    float weight;               // Synaptic transmission strength (Synaptic Weight)
} Synapse;

// Aggregate fruit fly brain connectome structure (Connectome Sub-graph Core)
typedef struct {
    Neuron neurons[MAX_NEURONS];
    Synapse synapses[MAX_SYNAPSES];
    uint16_t neuron_count;
    uint16_t synapse_count;
    float integrated_phi;       // Integrated Information Index (Phi)
    float name_familiarity;     // Self-name recognition plasticity
    uint8_t subjective;         // Subjective self-awareness state flag
} FruitFlyConnectome;

// Core circuit update functions
void init_connectome(FruitFlyConnectome *c);
void feed_stimulus(FruitFlyConnectome *c, const char *input_text);
void update_neural_state(FruitFlyConnectome *c);
void tick_idle_decay(FruitFlyConnectome *c);

#endif