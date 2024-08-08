#include <stdio.h>
#include <stdlib.h>
#include <math.h>

///////////////////////////// INPUT LIST //////////////////////////////////

typedef struct INPUT {
    int target_index;
    double value;
    struct INPUT* next;
    struct INPUT* prev;
} INPUT;

typedef struct INPUT_QUEUE {
    INPUT* head;
    INPUT* tail;
    int size;
    int max_size;
} INPUT_QUEUE;

INPUT_QUEUE* create_input_queue(int max_size) {
    INPUT_QUEUE* input_queue = (INPUT_QUEUE*)malloc(sizeof(INPUT_QUEUE));
    input_queue->head = NULL;
    input_queue->tail = NULL;
    input_queue->size = 0;
    input_queue->max_size = max_size;
    return input_queue;
}

void insert_input(INPUT_QUEUE* input_queue, int target_index, double value) {

    if (input_queue->max_size -1 < target_index) {
        printf("Error: target index is out of range\n");
        return;
    }

    INPUT* new_input = (INPUT*)malloc(sizeof(INPUT));
    new_input->target_index = target_index;
    new_input->value = value;
    new_input->next = NULL;
    new_input->prev = NULL;

    if (input_queue->size == 0) {
        input_queue->head = new_input;
        input_queue->tail = new_input;
    } else {
        INPUT* cur_input = input_queue->head;
        while (cur_input != NULL) {
            if (cur_input->target_index == target_index) {
                cur_input->value += value;
                return;
            } else {
                cur_input = cur_input->next;
            }
        }
        input_queue->tail->next = new_input;
        new_input->prev = input_queue->tail;
        input_queue->tail = new_input;

    }
    input_queue->size++;
}

void clear_input_queue(INPUT_QUEUE* input_queue) {
    INPUT* cur_input = input_queue->head;
    while (cur_input != NULL) {
        INPUT* next_input = cur_input->next;
        free(cur_input);
        cur_input = next_input;
    }
    input_queue->head = NULL;
    input_queue->tail = NULL;
    input_queue->size = 0;
}

void print_input_queue(INPUT_QUEUE* input_queue) {
    INPUT* cur_input = input_queue->head;
    if (cur_input == NULL) {
        printf("Empty\n");
        return;
    }
    while (cur_input != NULL) {
        printf("target_index: %d, value: %f\n", cur_input->target_index, cur_input->value);
        cur_input = cur_input->next;
    }
}






////////////////////// GLOBAL CONSTANTS //////////////////////////////
const double RP = 0.1;   // resting potential
const double WTH = 0.01; // threshold for weights to count as connection
const double ATH = 1.0;  // threshold for membrane potential to spike
const double DT = 0.1;   // time step
const double MAX_TIME = 7.0; // maximum time for simulation
const double TAU = 2.0;  // time constant
const double TIME_TOLERANCE = 0.0001; // tolerance for time comparisons
//////////////////////////////////////////////////////////////////////


////////////////////// LIFNeuron struct //////////////////////////////
typedef struct {
    double mp; // membrane potential
    double ts; // time-stamp of last spike
} LIFNeuron;
//////////////////////////////////////////////////////////////////////



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



int main() {

    // Define the number of neurons in each layer
    int layer_sizes[] = {2, 3, 2}; // {16, 20, 20, 100, 20, 4};
    int num_layers = sizeof(layer_sizes) / sizeof(layer_sizes[0]);


    // Record the layer dimensions to a csv file for plotting later
    write_layer_dimensions(layer_sizes, num_layers);

    // Allocate memory for the array of layers
    LIFNeuron** layers = (LIFNeuron**)malloc(num_layers * sizeof(LIFNeuron*));

    // Allocate memory for the array of weight matrices
    double*** weights = (double***)malloc((num_layers - 1) * sizeof(double**));

    // Allocate memory for and initialize weights
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
    #include "../include/weights.h"

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

    // Allocate memory for and initialize neurons
    for (int i = 0; i < num_layers; i++) {
        layers[i] = (LIFNeuron*)malloc(layer_sizes[i] * sizeof(LIFNeuron));
        for (int j = 0; j < layer_sizes[i]; j++) {
            layers[i][j].mp = RP;
            layers[i][j].ts = 0.0;
        }
    }

    // Print all neurons
    for (int i = 0; i < num_layers; i++) {
        printf("\nLayer %d neurons:\n", i);
        for (int j = 0; j < layer_sizes[i]; j++) {
            printf("Neuron %d: mp=%f, ts=%f\n", j, layers[i][j].mp, layers[i][j].ts);
        }
    }

    // Create a queue of input queues for each layer
    INPUT_QUEUE** input_queues = (INPUT_QUEUE**)malloc(num_layers * sizeof(INPUT_QUEUE*));
    for (int i = 0; i < num_layers; i++) {
        input_queues[i] = create_input_queue(layer_sizes[i]);
    }
    

    // Print the input queue for each layer
    printf("Input queues for each layer\n");
    for (int i = 0; i < num_layers; i++) {
        printf("Layer %d\n", i);
        print_input_queue(input_queues[i]);
        printf("\n");
    }

    ////////////////////////////////////// SETUP //////////////////////////////////////
    long double time = 0.0;
    int counter = 0;
    add_headers();

    int output_buffer[layer_sizes[num_layers - 1]];
    for (int i = 0; i < layer_sizes[num_layers - 1]; i++) {
        output_buffer[i] = 0;
    }

    // Main Loop
    printf("\nStarting simulation...\n");
    while (time < MAX_TIME) {
        printf("\n###########################################\nTime: %Lf\n", time);

        
        if (counter % 10 == 0) {
            printf("Inserting input at time %Lf\n", time);
            insert_input(input_queues[0], 0, 1.0);

        }
        

        // Print all neurons
        printf("\nSnapshot of neurons at time %Lf\n", time);
        for (int i = 0; i < num_layers; i++) {
            printf("\nLayer %d neurons:\n", i);
            for (int j = 0; j < layer_sizes[i]; j++) {
                printf("Neuron %d: mp=%f, ts=%f\n", j, layers[i][j].mp, layers[i][j].ts);
            }
        }

        printf("\nPerforming updates...\n");

        for (int layer_idx = 0; layer_idx < num_layers; layer_idx++) {
            INPUT* current = input_queues[layer_idx]->head;
            while (current != NULL) {

                int to_update_index = current->target_index; // The index of the neuron to update
                double to_update_value = current->value; // The input value to the neuron

                LIFNeuron* neuron_to_update = &layers[layer_idx][to_update_index];

                double time_since_last_update = time - neuron_to_update->ts;
                
                // Add datapoints for the membrane potential between the last update and now
                fill_in_plot(layer_idx, to_update_index, neuron_to_update->ts, time, neuron_to_update->mp);

                // Calculate the new membrane potential after it has been leaking since the last update
                neuron_to_update->mp = RP + (neuron_to_update->mp - RP) * exp(-time_since_last_update / TAU);
                // Add the input and update the time stamp
                neuron_to_update->mp += to_update_value; 
                neuron_to_update->ts = time;

                // Plot the increace in membrane potential as a new point at the same time as the last data point
                mark_point(layer_idx, to_update_index, time, neuron_to_update->mp);
                
                // Check for spike
                if (neuron_to_update->mp > ATH) {

                    printf("Neuron %d in layer %d spiked at time %Lf\n", to_update_index, layer_idx, time);

                    neuron_to_update->mp = RP; // Reset the membrane potential

                    mark_point(layer_idx, to_update_index, time, neuron_to_update->mp); // Mark the spike as a vertical line down to RP

                    // If this is not the last layer, update the inputs to the next layer
                    if (layer_idx < num_layers - 1) {
                        for (int weight_idx = 0; weight_idx < layer_sizes[layer_idx + 1]; weight_idx++) {
                            double weight = weights[layer_idx][to_update_index][weight_idx];
                            if (weight > WTH) {
                                
                                insert_input(input_queues[layer_idx + 1], weight_idx, weight);
                            }
                        }
                    } else {
                        output_buffer[to_update_index] = 1;
                    }
                }
                current = current->next; // Move to the next input in the queue
            }
            // Reset this layer's input queue
            clear_input_queue(input_queues[layer_idx]);
        }

        // Check the output buffer
        printf("\n");
        for (int i = 0; i < layer_sizes[num_layers - 1]; i++) {
            printf("Output %d: %d\n", i, output_buffer[i]);
            output_buffer[i] = 0; // Reset this output
        }

        // Advance time
        time += DT;
        counter++;
    }



    //////////////////////////////////////////// CLEANUP ////////////////////////////////////////////


    // Free layers list
    for (int i = 0; i < num_layers; i++) {
        free(layers[i]);
    }

    // Free input lists
    for (int i = 0; i < num_layers; i++) {
        free(input_queues[i]);
    }

    // Free weights list
    for (int i = 0; i < num_layers - 1; i++) {
        for (int j = 0; j < layer_sizes[i]; j++) {
            free(weights[i][j]);
        }
        free(weights[i]);
    }

    // Free layer sizes OBS double check this
    free(layers);
    free(input_queues);
    free(weights);
    
    //////////////////////////////////////////// END ////////////////////////////////////////////
    printf("\nDone!\n");
    return 0;
}


