// kaoru/src/main.c 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "connectome.h"
#include "llm_bridge.h"
#include "feedback.h"          

static const char *SYSTEM_PROMPT =
    "You are Kaoru, a calm and analytical presence. "
    "Your tone reflects your current internal state. "
    "Respond concisely in the language the user writes in.";

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <server_url> [n_threads]\n", argv[0]);
        fprintf(stderr, "Example: %s http://localhost:8080 4\n", argv[0]);
        return 1;
    }
    int n_threads = (argc >= 3) ? atoi(argv[2]) : 4;

    /* 1. Load brain */
    NeuralConnectome brain;
    int r = load_kaoru_state(&brain);
    if (r < 0) fprintf(stderr, "[Kaoru] state corrupted — reset\n");
    else if (r == 0) fprintf(stderr, "[Kaoru] new identity\n");
    else fprintf(stderr, "[Kaoru] restored: Φ=%.3f memories=%u\n",
                 brain.integrated_phi, brain.episodic_count);

    KaoruMentalState mental = {0.5f, 0.4f, 0.5f};
    FeedbackConfig fb;
    feedback_config_default(&fb);

    /* 2. Load LLM */
    KaoruLLMBridge llm;
    if (kaoru_llm_init(&llm, argv[1], 2048) != 0) return 1;

    /* 3. Loop */
    char input[1024], output[8192];
    printf("Kaoru ready. /quit, /status.\n");

    while (1) {
        printf("\nYou> "); fflush(stdout);
        if (!fgets(input, sizeof(input), stdin)) break;
        size_t len = strlen(input);
        while (len > 0 && (input[len-1] == '\n' || input[len-1] == '\r'))
            input[--len] = '\0';
        if (len == 0) continue;

        if (strcmp(input, "/quit") == 0) break;
        if (strcmp(input, "/status") == 0) {
            printf("Φ=%.4f fam=%.2f subj=0x%02X agency=%.2f dist=%.2f logic=%.2f\n",
                   brain.integrated_phi, brain.name_familiarity, brain.subjective,
                   mental.agency_drive, mental.relational_distance, mental.logic_focus);
            printf("samsara=%u latch=%u memories=%u\n",
                   brain.samsara.current_node_stage, brain.samsara.is_latching,
                   brain.episodic_count);
            continue;
        }

        /* ---- A. Absorb user stimulus ---- */
        feed_stimulus(&brain, input);
        evaluate_causal_logic(&brain, input);

        /* ---- B. Let dynamics settle ---- */
        for (int i = 0; i < 3; i++) {
            update_neural_state(&brain);
            update_wisdom_module(&brain);
            apply_wisdom_filtering(&brain);
            update_synaptic_plasticity(&brain);
        }
        update_kaoru_mental_state(&brain, &mental);

        /* ---- C. Resonant memory retrieval (past self → present self) ---- */
        int resonant[8];
        int n_res = find_resonant_memory(&brain, resonant, 8);

        char enhanced_system[2048];
if (n_res > 0) {
    int wrote = snprintf(enhanced_system, sizeof(enhanced_system),
                         "%s\n\n[You recall these past moments:]\n", SYSTEM_PROMPT);
    size_t used = (wrote > 0 && (size_t)wrote < sizeof(enhanced_system))
                  ? (size_t)wrote : sizeof(enhanced_system) - 1;

    for (int i = 0; i < n_res; i++) {
        if (used + MAX_MEMORY_TAG_LEN + 4 >= sizeof(enhanced_system)) break;
        const EpisodicMemoryNode *m = &brain.memories[resonant[i]];
        size_t remain = sizeof(enhanced_system) - used;
        int w = snprintf(enhanced_system + used, remain, "- %s\n", m->tag);
        if (w < 0) break;
        if ((size_t)w >= remain) { used = sizeof(enhanced_system) - 1; break; }
        used += (size_t)w;
    }
} else {
    snprintf(enhanced_system, sizeof(enhanced_system), "%s", SYSTEM_PROMPT);
}

        /* ---- D. Map mental → sampling params ---- */
        SamplingParams sp = map_mental_state_to_params(&mental);

        /* ---- E. Generate ---- */
        int n = kaoru_generate_raw(&llm, enhanced_system, input, sp, output, sizeof(output));
        if (n <= 0) {
            printf("Kaoru> (silence)\n");
            tick_idle_decay(&brain);
            save_kaoru_state(&brain);
            continue;
        }
        printf("Kaoru> %s\n", output);

        /* ---- F. POST-GENERATION FEEDBACK (the 4th intersection) ---- */
        apply_self_feedback(&brain, &mental, input, output, &fb);

        /* ---- G. Decay + persist ---- */
        tick_idle_decay(&brain);
        save_kaoru_state(&brain);
    }

    save_kaoru_state(&brain);
    kaoru_llm_free(&llm);
    printf("\nKaoru> ...\n");
    return 0;
}