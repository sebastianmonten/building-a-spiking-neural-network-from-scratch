#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// GLOBAL VARIABLES
const double RP = 0.1; // resting potential
const double WTH = 0.1; // threshold for weights to count as connection
const double DT = 0.1; // time step

// LIFNeuron struct
typedef struct {
    double mp; // membrane potential
    double ts; // time-stamp of last spike
    double* weights_ptr; // pointer to weights matrix
    struct LIFNeuron* next_layer_ptr; // pointer to next layer array
} LIFNeuron;



void run(LIFNeuron** layers, double*** weights, int* layer_sizes, int num_layers) {
    printf("\nRunning network!\n");
    long double time = 0.0;
    while (time < 10) {
        time += DT;
    }
}



int main() {
    // Define the number of neurons in each layer
    int layer_sizes[] = {2, 3, 2};
    int num_layers = sizeof(layer_sizes) / sizeof(layer_sizes[0]);

    // Allocate memory for the array of layers
    LIFNeuron** layers = (LIFNeuron**)malloc(num_layers * sizeof(LIFNeuron*));

    // Allocate memory for the array of weight matrices
    double*** weights = (double***)malloc((num_layers - 1) * sizeof(double**));

    // Allocate memory for each layer and weigh matrix. also initialize the neurons
    for (int i = 0; i < num_layers - 1; i++) {
        weights[i] = (double**)malloc(layer_sizes[i] * sizeof(double*));
        for (int j = 0; j < layer_sizes[i]; j++) {
            weights[i][j] = (double*)malloc(layer_sizes[i + 1] * sizeof(double));
            for (int k = 0; k < layer_sizes[i + 1]; k++) {
                weights[i][j][k] = 0.0; // You can initialize weights as needed
            }
        }
    }

    // manually setting some weights
    weights[0][0][0] = 0.5;

    // print all weights
    for (int i = 0; i < num_layers - 1; i++) {
        printf("\nLayer %d to Layer %d weights:\n", i, i + 1);
        for (int j = 0; j < layer_sizes[i]; j++) {
            printf("Neuron %d: ", j);
            for (int k = 0; k < layer_sizes[i + 1]; k++) {
                printf("%f ", weights[i][j][k]);
            }
            printf("\n");
        }
    }

    for (int i = 0; i < num_layers; i++) {
        layers[i] = (LIFNeuron*)malloc(layer_sizes[i] * sizeof(LIFNeuron));
        for (int j = 0; j < layer_sizes[i]; j++) {
            layers[i][j].mp = RP;
            layers[i][j].ts = 0.0;
            layers[i][j].weights_ptr = &(weights[i][j]); // You can allocate and initialize weights as needed
            if (i < num_layers - 1) {
                layers[i][j].next_layer_ptr = &layers[i+1]; // Set up pointers to the next layer as needed
            } else {
                layers[i][j].next_layer_ptr = NULL;
            }
        }
    }

    // print all neurons, including if they have a next layer
    for (int i = 0; i < num_layers; i++) {
        printf("\nLayer %d:\n", i);
        for (int j = 0; j < layer_sizes[i]; j++) {
            printf("Neuron %d: mp = %f, ts = %f, next_layer_ptr = %p\n", j, layers[i][j].mp, layers[i][j].ts, layers[i][j].next_layer_ptr);
        }
    }



    run(layers, weights, layer_sizes, num_layers);



    // Free memory
    for (int i = 0; i < num_layers - 1; i++) {
        for (int j = 0; j < layer_sizes[i]; j++) {
            free(weights[i][j]);
        }
        free(weights[i]);
    }
    free(weights);

    for (int i = 0; i < num_layers; i++) {
        free(layers[i]);
    }

    printf("\nDone!\n");
    return 0;
}