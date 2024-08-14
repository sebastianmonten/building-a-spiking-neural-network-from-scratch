#pragma once

#include "../include/main.h"

// Function to convert a weight to a float value between 0.1 and 1.0
float convert_weight(int16_t weight, int lower_bound, int upper_bound) {
    return 0.1 + (float)(weight - lower_bound) / (upper_bound - lower_bound) * (1.0 - 0.1);
}


// Function to load weights from a binary file
void load_weights(const char* filename, int num_layers, int* layer_sizes, int16_t**** weights) {
    printf("Attempting to open file: %s\n", filename);



    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open weights file");
        exit(EXIT_FAILURE);
    }

    *weights = (int16_t***)malloc((num_layers - 1) * sizeof(int16_t**));
    for (int i = 0; i < num_layers - 1; i++) {
        (*weights)[i] = (int16_t**)malloc(layer_sizes[i] * sizeof(int16_t*));
        for (int j = 0; j < layer_sizes[i]; j++) {
            (*weights)[i][j] = (int16_t*)malloc(layer_sizes[i + 1] * sizeof(int16_t));
            for (int k = 0; k < layer_sizes[i + 1]; k++) {
                fread(&(*weights)[i][j][k], sizeof(int16_t), 1, file);
            }
        }
    }

    fclose(file);
}