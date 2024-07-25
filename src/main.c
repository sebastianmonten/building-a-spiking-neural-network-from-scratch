#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    double tau; // membrane time constant
    double rp;  // resting potential
    double ath; // activation threshold
    double mp;  // membrane potential    
} LIFNeuron;

bool LIF_update(LIFNeuron *self, double input, double dt) {
    double dmp = (-(self->mp - self->rp) + input) * (dt / self->tau);
    self->mp += dmp;

    if (self->mp >= self->ath) {
        self->mp = self->rp; // reset to resting potential
        printf("\nSpike!\n");
        return true; // spike
    } else {
        return false; // no spike
    }
}


int main() {

    LIFNeuron n0 = {6.0, 0.01, 0.1, 0.01};

    LIFNeuron * layer1 [3];
    double weights1 [3] = {0.3, 0.5, 1.0};

    // Step 3: Allocate memory for each LIFNeuron and assign it to the array
    for (int i = 0; i < 3; i++) {
        layer1[i] = (LIFNeuron *)malloc(sizeof(LIFNeuron));
        if (layer1[i] == NULL) {
            printf("Memory allocation failed\n");
            return 1;
        }

        layer1[i]->tau = 6.0;
        layer1[i]->rp = 0.01;
        layer1[i]->ath = 0.06;
        layer1[i]->mp = 0.01;
    }
    

    FILE *fptr;

    // Open a file in writing mode
    fptr = fopen("bin/tmp_output.csv", "w+");

    // Write some text to the file
    fprintf(fptr, "Time,MemPot0,MemPot1,MemPot2,MemPot3\n");
    
    double dt = 0.1;
    double t = 0.0;
    double inputs[100] = {0};

    inputs[0] = 2;
    inputs[17] = 2;
    inputs[30] = 2;
    inputs[50] = 2;
    inputs[56] = 2;
    inputs[64] = 2;
    inputs[70] = 2;
    inputs[80] = 2;

    fprintf(fptr, "%f,%f,%f,%f,%f\n", t, n0.mp, layer1[0]->mp, layer1[1]->mp, layer1[2]->mp);
    for (int i = 0; i < 100; i++) {
        t += dt;
        bool spike0 = LIF_update(&n0, inputs[i], dt);
        if (spike0) {
            bool spike1 = LIF_update(layer1[0], 2.0*weights1[0], dt);
            bool spike2 = LIF_update(layer1[1], 2.0*weights1[1], dt);
            bool spike3 = LIF_update(layer1[2], 2.0*weights1[2], dt);
        } else {
            LIF_update(layer1[0], 0.0, dt);
            LIF_update(layer1[1], 0.0, dt);
            LIF_update(layer1[2], 0.0, dt);
        }
        fprintf(fptr, "%f,%f,%f,%f,%f\n", t, n0.mp, layer1[0]->mp, layer1[1]->mp, layer1[2]->mp);
    }

    // // Close the file
    // fclose(fptr);


    // Don't forget to free the allocated memory
    for (int i = 0; i < 3; i++) {
        free(layer1[i]);
    }

    printf("\nDone!\n");
    return 0;
}