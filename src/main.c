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

    LIFNeuron n1 = {6.0, 0.0, 0.06, 0.0};

    FILE *fptr;

    // Open a file in writing mode
    fptr = fopen("bin/tmp_output.csv", "w+");

    // Write some text to the file
    fprintf(fptr, "Time,Membrane Potential\n");
    
    double dt = 0.1;
    double t = 0.0;
    double inputs[100];
    inputs[0] = 1.8;
    inputs[12] = 1.8;
    inputs[30] = 1.8;
    inputs[50] = 1.8;
    inputs[70] = 1.8;
    fprintf(fptr, "%f,%f\n", t, n1.mp);
    for (int i = 0; i < 100; i++) {
        t += dt;
        bool spike = LIF_update(&n1, inputs[i], dt);
        fprintf(fptr, "%f,%f\n", t, n1.mp);
    }

    // Close the file
    fclose(fptr);

    printf("\nDone!\n");
    return 0;
}