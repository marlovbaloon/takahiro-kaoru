// kaoru/include/connectome.h
#ifndef CONNECTOME_H
#define CONNECTOME_H

#include <stdint.h>
#include <stddef.h>

#define MAX_NEURONS            64
#define MAX_SYNAPSES           256
#define MAX_EPISODIC_MEMORIES  64
#define MAX_MEMORY_TAG_LEN     96

/* ============ Behavioral Vector ============ */
typedef struct {
    float agency_drive;
    float relational_distance;
    float logic_focus;
} KaoruMentalState;

/* ============ 52 Cetasika ============ */
typedef struct {
    float contact, feeling, perception, volition, concentration,
          mental_vitality, attention;
    float initial_thought, sustained_evaluation, decision, effort,
          enthusiasm, intent_to_act;
    float delusion, shamelessness, recklessness, restlessness;
    float greed_attachment, conceit_pride, wrong_view_bias;
    float aversion_hostility, envy, possessiveness, regret_remorse;
    float sloth, torpor, skepticism_doubt;
    float trust_conviction, mindfulness_retention, self_respect_dignity,
          prudent_dread, non_attachment, non_aversion_composure,
          equanimity_neutrality;
    float wisdom_logic;
} CetasikaMatrix;

typedef enum {
    STIM_SOURCE_USER = 0,
    STIM_SOURCE_SELF = 1,
    STIM_SOURCE_ENV  = 2
} StimulusSource;

typedef enum {
    STIM_AUDITORY_NAME = 0,
    STIM_AUDITORY_GENERAL,
    STIM_TACTILE_TOUCH,
    STIM_ENVIRONMENTAL
} StimulusType;

typedef struct {
    StimulusType type;
    float intensity;
    const char *payload;
} Stimulus;

/* ============ Neuron (fields used by connectome.c) ============ */
typedef struct {
    uint8_t id;
    uint8_t cortical_layer;      /* 0=Sensory, 1=Limbic, 2=Association, 3=Motor */
    float   membrane_potential;
    float   threshold;
    float   decay_rate;
    uint8_t refractory_ticks;
} Neuron;

typedef struct {
    uint8_t pre_neuron_id;
    uint8_t post_neuron_id;
    float   weight;
} Synapse;

typedef struct {
    uint8_t current_node_stage;
    float   loop_momentum;
    uint8_t is_latching;
} SamsaraState;

/* ============ Episodic Memory (must precede NeuralConnectome) ============ */
typedef struct {
    uint64_t memory_id;
    int64_t  created_at;            /* unix epoch, seconds */
    float    salience;              /* [0.0, 1.0] */
    float    phi_at_encoding;
    uint8_t  samsara_stage_at_encoding;
    CetasikaMatrix cetasika_snapshot;
    char     tag[MAX_MEMORY_TAG_LEN];
} EpisodicMemoryNode;

/* ============ Full Connectome (state that lives in RAM) ============ */
typedef struct {
    Neuron neurons[MAX_NEURONS];
    Synapse synapses[MAX_SYNAPSES];
    uint16_t neuron_count;
    uint16_t synapse_count;

    float    integrated_phi;
    float    name_familiarity;
    uint8_t  subjective;

    CetasikaMatrix cetasika;
    SamsaraState   samsara;

    /* Episodic layer */
    uint8_t  episodic_count;
    uint64_t next_memory_id;
    EpisodicMemoryNode memories[MAX_EPISODIC_MEMORIES];
} NeuralConnectome;

/* ============ File Snapshot (mirrors on-disk format v2) ============ */
typedef struct {
    uint32_t magic_header;      /* "KAOR" */
    uint32_t version;
    uint64_t checksum;          /* FNV-1a over everything after this field */

    float    integrated_phi;
    float    name_familiarity;
    uint8_t  subjective;

    CetasikaMatrix cetasika;
    SamsaraState   samsara;

    uint16_t neuron_count;
    uint16_t synapse_count;

    float   neuron_potentials[MAX_NEURONS];
    uint8_t neuron_refractory[MAX_NEURONS];

    Synapse synapses[MAX_SYNAPSES];

    uint8_t  episodic_count;
    uint64_t next_memory_id;
    EpisodicMemoryNode memories[MAX_EPISODIC_MEMORIES];
} KaoruMindSnapshot;

/* ============ API ============ */
/* connectome.c */
void init_connectome(NeuralConnectome *c);
void feed_stimulus(NeuralConnectome *c, const char *input_text);
void feed_stimulus_sourced(NeuralConnectome *c, const char *text,
                           float weight, StimulusSource source);
void update_neural_state(NeuralConnectome *c);
void update_cetasika_matrix(NeuralConnectome *c);
void update_samsara_loop(NeuralConnectome *c);
void update_kaoru_mental_state(NeuralConnectome *c, KaoruMentalState *state);
void update_synaptic_plasticity(NeuralConnectome *c);
void tick_idle_decay(NeuralConnectome *c);

/* wisdom.c */
void evaluate_causal_logic(NeuralConnectome *c, const char *payload);
void update_wisdom_module(NeuralConnectome *c);
void apply_wisdom_filtering(NeuralConnectome *c);

/* memory.c */
float resonance_score(const NeuralConnectome *c, const EpisodicMemoryNode *m);
int   find_resonant_memory(const NeuralConnectome *c, int *out, int max_results);
void  encode_episodic_memory(NeuralConnectome *c, const char *tag);
void  consolidate_memories(NeuralConnectome *c);

/* persistence.c */
int save_kaoru_state(const NeuralConnectome *c);
int load_kaoru_state(NeuralConnectome *c);

#endif