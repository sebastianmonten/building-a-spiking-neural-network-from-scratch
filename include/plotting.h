#pragma once
#include "../include/main.h"

///////////////////////////// PLOTTING //////////////////////////////////
const char* NEURON_DATA_FILE_NAME = "tmp/neuron_data.csv";
const char* LAYER_DIMENSIONS_FILE_NAME = "tmp/layer_dimensions.csv";

void mark_point(int layer, int neuron, double time, double potential) {
    FILE *file = fopen(NEURON_DATA_FILE_NAME, "a");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    // Make an entry in the CSV file
    fprintf(file, "%d,%d,%f,%f\n", layer, neuron, time, potential);
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

void fill_in_plot(int layer, int neuron, double time_start, double time_end, double potential) {
    FILE *file = fopen(NEURON_DATA_FILE_NAME, "a");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    double time_i = time_start;
    while(time_i < time_end - TIME_TOLERANCE) {
        time_i += DT;
        potential += -(potential - RP) * DT / TAU;
        mark_point(layer, neuron, time_i, potential);
        
    }
    fclose(file);
}

void write_layer_dimensions(int* layer_sizes, int num_layers) {
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