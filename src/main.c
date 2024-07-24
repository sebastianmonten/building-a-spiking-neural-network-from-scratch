#include <stdio.h>
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

    LIFNeuron n1 = {6.0, 0.01, 0.06, 0.01};
    LIFNeuron n2 = {6.0, 0.01, 0.06, 0.01};

    FILE *fptr;

    // Open a file in writing mode
    fptr = fopen("bin/tmp_output.csv", "w+");

    // Write some text to the file
    fprintf(fptr, "Time,MemPot 1, MemPot 2\n");
    
    double dt = 0.1;
    double t = 0.0;
    double inputs[100] = {0};

    inputs[0] = 2;
    inputs[30] = 2;
    inputs[50] = 2;
    inputs[80] = 2;

    fprintf(fptr, "%f,%f,%f\n", t, n1.mp, n2.mp);
    for (int i = 0; i < 100; i++) {
        t += dt;
        bool spike1 = LIF_update(&n1, inputs[i], dt);
        if (spike1) {
            bool spike2 = LIF_update(&n2, 2.0, dt);
        } else {
            LIF_update(&n2, 0.0, dt);
        }
        fprintf(fptr, "%f,%f,%f\n", t, n1.mp, n2.mp);
    }

    // Close the file
    fclose(fptr);

    printf("\nDone!\n");
    return 0;
}