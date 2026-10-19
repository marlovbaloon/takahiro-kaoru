// test/test_brain_only.c
#include "connectome.h"
#include <stdio.h>
#include <string.h>

/* ============ Print helpers ============ */
static void print_separator(const char *title) {
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("  %s\n", title);
    printf("═══════════════════════════════════════════════════════════════\n");
}

static void print_mental(const KaoruMentalState *m) {
    printf("  KaoruMentalState:\n");
    printf("    agency_drive       = %.4f\n", m->agency_drive);
    printf("    relational_distance= %.4f\n", m->relational_distance);
    printf("    logic_focus        = %.4f\n", m->logic_focus);
}

static void print_brain(const NeuralConnectome *b, const char *label) {
    printf("\n  ── %s ──\n", label);
    printf("    name_familiarity   = %.4f\n", b->name_familiarity);
    printf("    integrated_phi     = %.4f\n", b->integrated_phi);
    printf("    subjective         = 0x%02X (%3d)\n", b->subjective, b->subjective);
    printf("    samsara_stage      = %u   (latch=%u, momentum=%.3f)\n",
           b->samsara.current_node_stage,
           b->samsara.is_latching,
           b->samsara.loop_momentum);
    printf("    ── Cetasika (subset) ──\n");
    printf("    wisdom_logic       = %.4f\n", b->cetasika.wisdom_logic);
    printf("    aversion_hostility = %.4f\n", b->cetasika.aversion_hostility);
    printf("    conceit_pride      = %.4f\n", b->cetasika.conceit_pride);
    printf("    equanimity         = %.4f\n", b->cetasika.equanimity_neutrality);
    printf("    mindfulness        = %.4f\n", b->cetasika.mindfulness_retention);
    printf("    restlessness       = %.4f\n", b->cetasika.restlessness);
    printf("    delusion           = %.4f\n", b->cetasika.delusion);
    printf("    decision           = %.4f\n", b->cetasika.decision);
}

static void print_neuron_summary(const NeuralConnectome *b, const char *label) {
    float total = 0, max_pot = 0;
    int firing = 0;
    for (int i = 0; i < b->neuron_count; i++) {
        float p = b->neurons[i].membrane_potential;
        total += p;
        if (p > max_pot) max_pot = p;
        if (p >= b->neurons[i].threshold) firing++;
    }
    printf("\n  ── Neurons (%s) ──\n", label);
    printf("    total_potential = %.3f\n", total);
    printf("    max_potential   = %.3f\n", max_pot);
    printf("    firing_count    = %d / %d\n", firing, b->neuron_count);
    printf("    avg_potential   = %.3f\n", total / b->neuron_count);
}

/* ============ Test scenario ============ */
static void run_test(NeuralConnectome *brain, KaoruMentalState *mental,
                     const char *input, int feed_times, int ticks_between) {
    printf("\n  ▶ feed_stimulus('%s') x%d, then %d neural ticks each\n",
           input, feed_times, ticks_between);
    
    for (int i = 0; i < feed_times; i++) {
        feed_stimulus(brain, input);
        for (int j = 0; j < ticks_between; j++) {
            update_neural_state(brain);
            update_wisdom_module(brain);
            update_synaptic_plasticity(brain);
        }
        update_kaoru_mental_state(brain, mental);
    }
}

/* ============ Main ============ */
int main(void) {
    NeuralConnectome brain;
    init_connectome(&brain);
    
    KaoruMentalState mental = {0.5f, 0.4f, 0.5f};
    
    /* ─────────────────────────────────────────────── */
    print_separator("TEST 1: Initial state (pristine)");
    print_brain(&brain, "Initial");
    print_mental(&mental);
    print_neuron_summary(&brain, "Initial");
    
    /* ─────────────────────────────────────────────── */
    print_separator("TEST 2: Neutral input ('hello')");
    run_test(&brain, &mental, "hello", 1, 3);
    print_brain(&brain, "After 'hello'");
    print_mental(&mental);
    print_neuron_summary(&brain, "After 'hello'");
    
    /* ─────────────────────────────────────────────── */
    print_separator("TEST 3: Name input ('かおる') x1");
    run_test(&brain, &mental, "かおる", 1, 3);
    print_brain(&brain, "After 'かおる' x1");
    print_mental(&mental);
    print_neuron_summary(&brain, "After 'かおる' x1");
    
    /* ─────────────────────────────────────────────── */
    print_separator("TEST 4: Name input ('かおる') x5 (accumulation)");
    run_test(&brain, &mental, "かおる", 5, 3);
    print_brain(&brain, "After 'かおる' x5 total");
    print_mental(&mental);
    print_neuron_summary(&brain, "After 'かおる' x5");
    
    /* ─────────────────────────────────────────────── */
    print_separator("TEST 5: Name input ('かおる') x10 (more accumulation)");
    run_test(&brain, &mental, "かおる", 10, 3);
    print_brain(&brain, "After 'かおる' x15 total");
    print_mental(&mental);
    print_neuron_summary(&brain, "After 'かおる' x15");
    
    /* ─────────────────────────────────────────────── */
    print_separator("TEST 6: Heavy name input ('かおるかおるかおる')");
    run_test(&brain, &mental, "かおるかおるかおる", 3, 3);
    print_brain(&brain, "After heavy input");
    print_mental(&mental);
    print_neuron_summary(&brain, "After heavy input");
    
    /* ─────────────────────────────────────────────── */
    print_separator("TEST 7: Idle decay (20 ticks, no input)");
    for (int i = 0; i < 20; i++) {
        tick_idle_decay(&brain);
        update_neural_state(&brain);
    }
    print_brain(&brain, "After idle decay");
    print_mental(&mental);
    print_neuron_summary(&brain, "After idle decay");
    
    /* ─────────────────────────────────────────────── */
    print_separator("DIFF SUMMARY");
    printf("\n  What to observe:\n");
    printf("  ──────────────────────────────────────────────\n");
    printf("  1. name_familiarity:  should INCREASE with 'かおる' inputs\n");
    printf("     - Initial: 0.0\n");
    printf("     - After 'かおる' x1:  ~0.05\n");
    printf("     - After 'かおる' x15: ~0.50+\n");
    printf("  2. subjective:  should INCREASE with familiarity + phi\n");
    printf("     - Depends on: familiarity*120 + phi*80 + samsara*5\n");
    printf("  3. conceit_pride:  should INCREASE with name input\n");
    printf("     - '+0.3 * weight' per 'かおる' stimulus\n");
    printf("  4. aversion_hostility:  should INCREASE with name input\n");
    printf("     - 'polite coldness' effect: '+0.2 * weight'\n");
    printf("  5. integrated_phi:  changes with neural firing\n");
    printf("  6. After decay:  familiarity + phi should DECREASE\n");
    printf("  7. mental state should REFLECT brain changes:\n");
    printf("     - relational_distance increases (aversion -> distance)\n");
    printf("     - logic_focus reflects wisdom\n");
    printf("\n");
    return 0;
}