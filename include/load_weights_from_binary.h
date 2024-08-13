#pragma once

#include "../include/main.h"

void load_weights_from_binary(double ***weights, int *layer_sizes, int num_layers, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_layers - 1; i++) {
        for (int j = 0; j < layer_sizes[i]; j++) {
            for (int k = 0; k < layer_sizes[i + 1]; k++) {
                fread(&weights[i][j][k], sizeof(double), 1, file);
            }
        }
    }

    fclose(file);
}
