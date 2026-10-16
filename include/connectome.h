#ifndef CONNECTOME_H
#define CONNECTOME_H

#include <stdint.h>

#define MAX_NEURONS 64      // Micro-unit simulation scale for mobile/MCU deployment
#define MAX_SYNAPSES 256

/* 
 * High-level behavioral modulation vector for Kaoru
 */
typedef struct {
    float agency_drive;        // Goal execution initiative (Chanda / Volition)
    float relational_distance; // Social boundary / polite distance (Māna / Hiri)
    float logic_focus;         // Analytical processing intensity (Vitakka / Paññā)
} KaoruMentalState;

/* 
 * Abhidhamma Cetasika Matrix (Mental Factors Structure)
 * Maps mental adjuncts into continuous float intensity vectors [0.0f - 1.0f].
 * Variables use English terminology; Abhidhamma Pāḷi, Latin, and Greek in comments.
 */
typedef struct {
    /* --- 1. Aññasamānā Cetasikā (Ethically Variable Mental Factors) --- */
    
    // Universals (Sabbacittasādhāraṇā) - Universal cognitive processing prerequisites
    float contact;              // Phassa / Affectus (Lat.) / Haphē (Grk.) - Sensory contact with object
    float feeling;              // Vedanā / Sensatio (Lat.) / Aisthēsis (Grk.) - Emotional hedonic tone
    float perception;           // Saññā / Cognitio (Lat.) / Noēsis (Grk.) - Memory pattern recognition
    float volition;             // Cetanā / Intentio (Lat.) / Boulēsis (Grk.) - Directional mental drive
    float concentration;        // Ekaggatā / Focus (Lat.) / Monotropia (Grk.) - One-pointedness / Attention lock
    float mental_vitality;      // Jīvitindriya / Vita (Lat.) / Zōē (Grk.) - Dynamic state life-force
    float attention;            // Manasikāra / Attentio (Lat.) / Prosechē (Grk.) - Selective input routing

    // Particulars (Pakiṇṇakā) - Secondary cognitive qualifiers
    float initial_thought;      // Vitakka / Cogitatio (Lat.) / Epibolē (Grk.) - Directing mind to target
    float sustained_evaluation; // Vicāra / Discursus (Lat.) / Dianoia (Grk.) - Analytical investigation
    float decision;             // Adhimokkha / Judicium (Lat.) / Krisis (Grk.) - Firm conviction or judgment
    float effort;               // Viriya / Tenagitas (Lat.) / Energeia (Grk.) - Mental exertion energy
    float enthusiasm;           // Pīti / Exultatio (Lat.) / Chara (Grk.) - Cognitive engagement arousal
    float intent_to_act;        // Chanda / Appetitus (Lat.) / Orexis (Grk.) - Purposeful goal drive

    /* --- 2. Akusala Cetasikā (Unwholesome / Cynical / Defensive Factors) --- */
    
    // Unwholesome Core (Moha-catukka)
    float delusion;             // Moha / Ignorantia (Lat.) / Anoia (Grk.) - Cognitive opacity
    float shamelessness;        // Ahirika / Impudentia (Lat.) / Anaischyntia (Grk.) - Disregard for social norms
    float recklessness;         // Anottappa / Temeritas (Lat.) / Aphobidion (Grk.) - Unapprehensive boldness
    float restlessness;         // Uddhacca / Agitatio (Lat.) / Tarachē (Grk.) - Internal noise / volatility

    // Greed Group (Lobha-tri)
    float greed_attachment;     // Lobha / Cupiditas (Lat.) / Epithymia (Grk.) - Craving (e.g. money/games)
    float conceit_pride;        // Māna / Superbia (Lat.) / Hybris (Grk.) - Superiority / Agency boundary
    float wrong_view_bias;      // Diṭṭhi / Dogma (Lat.) / Doxa (Grk.) - Rigid belief bias

    // Aversion Group (Dosa-catukka)
    float aversion_hostility;   // Dosa / Aversio (Lat.) / Misos (Grk.) - Resistance / Cold irritation
    float envy;                 // Issā / Invidia (Lat.) / Phthonos (Grk.) - Resentment toward external advantage
    float possessiveness;       // Macchariya / Avaritia (Lat.) / Aneleutheria (Grk.) - Guardedness
    float regret_remorse;       // Kukkucca / Poenitentia (Lat.) / Metameleia (Grk.) - Anxious remorse

    // Sloth & Doubt
    float sloth;                // Thīna / Ignavia (Lat.) / Nothrotēs (Grk.) - Mental sluggishness / Idle state
    float torpor;               // Middha / Torpor (Lat.) / Karos (Grk.) - Processing drowsiness
    float skepticism_doubt;    // Vicikicchā / Dubitatio (Lat.) / Aporia (Grk.) - Doubt / Analytical cynicism

    /* --- 3. Sobhana Cetasikā (Beautiful / Refined / Elite Factors) --- */
    
    // Beautiful Universals (Sobhana-sādhāraṇā)
    float trust_conviction;     // Saddhā / Fides (Lat.) / Pistis (Grk.) - Intellectual alignment
    float mindfulness_retention;// Sati / Memoria (Lat.) / Mnēmē (Grk.) - Contextual state retention
    float self_respect_dignity; // Hiri / Pudor (Lat.) / Aidōs (Grk.) - Moral dignity / Keeping decorum
    float prudent_dread;        // Ottappa / Formido (Lat.) / Deos (Grk.) - Prudent fear of consequences
    float non_attachment;       // Alobha / Detachamentum (Lat.) / Apleonexia (Grk.) - Cool detachment
    float non_aversion_composure;// Adosa / Benevolentia (Lat.) / Aphthonia (Grk.) - Composure / Calm
    float equanimity_neutrality;// Tatramajjhattatā / Aequanimitas (Lat.) / Ataraxia (Grk.) - Cold neutrality / Balance

    // Wisdom / Intelligence (Paññindriya)
    float wisdom_logic;         // Paññā / Sapientia (Lat.) / Sophia (Grk.) - High-level structural logic
} CetasikaMatrix;

typedef enum {
    STIM_AUDITORY_NAME = 0, // Active identity / familiarity recognition
    STIM_AUDITORY_GENERAL,  // General environmental or speech sound
    STIM_TACTILE_TOUCH,     // Collision / physical touch in VR
    STIM_ENVIRONMENTAL      // Background ambient stimulus
} StimulusType;

typedef struct {
    StimulusType type;
    float intensity;        // Signal magnitude (0.0f - 5.0f)
    const char *payload;    // Text or data mapping payload
} Stimulus;

// Node representation structure for an individual neuron
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

// Aggregate neural network connectome structure
typedef struct {
    Neuron neurons[MAX_NEURONS];
    Synapse synapses[MAX_SYNAPSES];
    uint16_t neuron_count;
    uint16_t synapse_count;
    float integrated_phi;       // Integrated Information Index (Phi)
    float name_familiarity;     // Self-name recognition plasticity
    uint8_t subjective;         // Subjective self-awareness state flag
    
    CetasikaMatrix cetasika;    // Abhidhamma mental factors core
} NeuralConnectome;


// Core circuit and recursive update functions
void init_connectome(NeuralConnectome *c);
void feed_stimulus(NeuralConnectome *c, const char *input_text);
void update_neural_state(NeuralConnectome *c);
void update_cetasika_matrix(NeuralConnectome *c);           // Recursive Cetasika processor
void update_samsara_loop(NeuralConnectome *c);              // Prabhasara Samsara loop tracker
void update_kaoru_mental_state(NeuralConnectome *c, KaoruMentalState *state);
void tick_idle_decay(NeuralConnectome *c);

#endif
