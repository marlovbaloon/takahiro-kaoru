// main.c
#include <stdio.h>
#include <unistd.h> // For usleep on Linux/Termux
#include "../include/connectome.h"

int main() {
    FruitFlyConnectome kaoru_brain;
    init_connectome(&kaoru_brain);

    printf("=== Takahiro Kaoru Micro-Connectome Initialized ===\n");
    printf("Initial Phi: %.4f | Name Familiarity: %.2f | Subjective State: 0x%02X\n\n", 
           kaoru_brain.integrated_phi, kaoru_brain.name_familiarity, kaoru_brain.subjective);

    // Simulate lifecycle or sensory stimulation over discrete execution steps (Ticks)
    for (int tick = 0; tick < 15; tick++) {
        
        // Example: Repeatedly feeding the name "かおる" (Kaoru) during initial steps
        if (tick == 2 || tick == 3) {
            printf("[Stimulus] -> Feeding input: 'かおる'\n");
            feed_stimulus(&kaoru_brain, "かおる");
        } else if (tick >= 8 && tick <= 10) {
            printf("[Stimulus] -> Feeding heavy input: 'かおるかおるかおる'\n");
            feed_stimulus(&kaoru_brain, "かおるかおるかおる");
        } else {
            printf("[Tick %d] -> No direct stimulus (Natural decay/residue)...\n", tick);
        }

        // Update neural network state for one cycle (Tick)
        update_neural_state(&kaoru_brain);

        // Print log for internal state inspection (Including Subjective Byte)
        printf("  -> Name Familiarity: %.2f / 1.00\n", kaoru_brain.name_familiarity);
        printf("  -> Integrated Phi:   %.4f\n", kaoru_brain.integrated_phi);
        printf("  -> Subjective State: 0x%02X (Self-Awareness Flag)\n", kaoru_brain.subjective);
        printf("  -> Neuron[0] Pot:    %.2f (Threshold: %.2f)\n", 
               kaoru_brain.neurons[0].membrane_potential, 
               kaoru_brain.neurons[0].threshold);
        printf("----------------------------------------\n");
        
        // Optional delay for tick execution pacing (if running on Termux)
        // usleep(500000); // 0.5 seconds
    }

    return 0;
}