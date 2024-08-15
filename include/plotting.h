#pragma once
#include "../include/main.h"

///////////////////////////// PLOTTING //////////////////////////////////
const char* NEURON_DATA_FILE_NAME = "tmp/neuron_data.csv";
const char* LAYER_DIMENSIONS_FILE_NAME = "tmp/layer_dimensions.csv";

void mark_point(int layer, neuron_idx_t neuron, neuron_ts_t time, neuron_mp_t potential) {
    FILE *file = fopen(NEURON_DATA_FILE_NAME, "a");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    // Make an entry in the CSV file
    fprintf(file, "%d,%d,%d,%d\n", layer, neuron, time, potential);
    fclose(file);
}

void add_headers() {
    FILE *file = fopen(NEURON_DATA_FILE_NAME, "a");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }
    // Add headers to the CSV file
    fprintf(file, "layer,neuron,time,potential\n");
    fclose(file);
}

void fill_in_plot(int layer, neuron_idx_t neuron, neuron_ts_t time_start, neuron_ts_t time_end, neuron_mp_t potential) {
    FILE *file = fopen(NEURON_DATA_FILE_NAME, "a");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    decimal_temporary_t new_potential = (decimal_temporary_t) potential;

    neuron_ts_t time_i = time_start;
    while(time_i < time_end) {
        time_i += DT;
        new_potential += -(new_potential - ((decimal_temporary_t) RP)) * ((decimal_temporary_t) DT) / TAU;
        mark_point(layer, neuron, time_i, (neuron_mp_t) new_potential);
        
    }
    fclose(file);
}

void write_layer_dimensions(const neuron_idx_t * layer_sizes, int num_layers) {
    FILE *file = fopen(LAYER_DIMENSIONS_FILE_NAME, "a");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }
    fprintf(file, "neurons\n");
    for (int i = 0; i < num_layers; i++) {
        fprintf(file, "%d\n", layer_sizes[i]);
    }
    fclose(file);
}
//////////////////////////////////////////////////////////////////////