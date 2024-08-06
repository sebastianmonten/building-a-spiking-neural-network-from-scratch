#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

///////////////////////////// INPUT LIST //////////////////////////////////

typedef struct INPUT {
    int target_index;
    double value;
    struct INPUT* next;
    struct INPUT* prev;
} INPUT;

typedef struct INPUT_LIST {
    INPUT* head;
    INPUT* tail;
    int size;
    int max_size;
} INPUT_LIST;

INPUT_LIST* create_input_list(int max_size) {
    INPUT_LIST* input_list = (INPUT_LIST*)malloc(sizeof(INPUT_LIST));
    input_list->head = NULL;
    input_list->tail = NULL;
    input_list->size = 0;
    input_list->max_size = max_size;
    return input_list;
}

void insert_input(INPUT_LIST* input_list, int target_index, double value) {

    if (input_list->max_size -1 < target_index) {
        printf("Error: target index is out of range\n");
        return;
    }

    INPUT* new_input = (INPUT*)malloc(sizeof(INPUT));
    new_input->target_index = target_index;
    new_input->value = value;
    new_input->next = NULL;
    new_input->prev = NULL;

    if (input_list->size == 0) {
        input_list->head = new_input;
        input_list->tail = new_input;
    } else {
        INPUT* cur_input = input_list->head;
        while (cur_input != NULL) {
            if (cur_input->target_index == target_index) {
                cur_input->value += value;
                return;
            } else {
                cur_input = cur_input->next;
            }
        }
        input_list->tail->next = new_input;
        new_input->prev = input_list->tail;
        input_list->tail = new_input;

    }
    input_list->size++;
}

void clear_input_list(INPUT_LIST* input_list) {
    INPUT* cur_input = input_list->head;
    while (cur_input != NULL) {
        INPUT* next_input = cur_input->next;
        free(cur_input);
        cur_input = next_input;
    }
    input_list->head = NULL;
    input_list->tail = NULL;
    input_list->size = 0;
}

void print_input_list(INPUT_LIST* input_list) {
    INPUT* cur_input = input_list->head;
    if (cur_input == NULL) {
        printf("Empty\n");
        return;
    }

    while (cur_input != NULL) {
        printf("target_index: %d, value: %f\n", cur_input->target_index, cur_input->value);
        cur_input = cur_input->next;
    }
}



///////////////////////////// PLOTTING //////////////////////////////////

void mark_spike(int layer, int neuron, double time, double potential) {
    FILE *file = fopen("bin/neuron_data.csv", "a");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }
    fprintf(file, "%d,%d,%f,%f,spike\n", layer, neuron, time, potential);
    fclose(file);
}
//////////////////////////////////////////////////////////////////////




////////////////////// GLOBAL CONSTANTS //////////////////////////////
const double RP = 0.1;   // resting potential
const double WTH = 0.01; // threshold for weights to count as connection
const double ATH = 1.0;  // threshold for membrane potential to spike
const double DT = 1.0;   // time step
// LIFNeuron struct
typedef struct {
    double mp; // membrane potential
    double ts; // time-stamp of last spike
} LIFNeuron;
//////////////////////////////////////////////////////////////////////

int main() {

    // Define the number of neurons in each layer
    int layer_sizes[] = {16, 20, 20, 100, 20, 4};
    int num_layers = sizeof(layer_sizes) / sizeof(layer_sizes[0]);

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
    weights[0][0][0] = 0.5;
    weights[1][0][0] = 0.5;

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

    // Create a list of input lists for each layer
    INPUT_LIST** input_lists = (INPUT_LIST**)malloc(num_layers * sizeof(INPUT_LIST*));
    for (int i = 0; i < num_layers; i++) {
        input_lists[i] = create_input_list(layer_sizes[i]);
    }
    

    // Print the input list for each layer
    printf("Input lists for each layer\n");
    for (int i = 0; i < num_layers; i++) {
        printf("Layer %d\n", i);
        print_input_list(input_lists[i]);
        printf("\n");
    }

    ////////////////////////////////////// SETUP //////////////////////////////////////
    long double time = 0.0;
    int output_buffer[layer_sizes[num_layers - 1]];
    for (int i = 0; i < layer_sizes[num_layers - 1]; i++) {
        output_buffer[i] = 0;
    }

    // Main Loop
    printf("\nStarting simulation...\n");
    while (time < 7.0) {
        printf("\n###########################################\nTime: %Lf\n", time);

        // Introduce new input
        printf("\nIntroducing new input at time %Lf\n", time);
        insert_input(input_lists[0], 0, 1.0);
        insert_input(input_lists[0], 1, 1.0);
        

        // Print all neurons
        printf("\nSnapshot of neurons at time %Lf\n", time);
        for (int i = 0; i < num_layers; i++) {
            printf("\nLayer %d neurons:\n", i);
            for (int j = 0; j < layer_sizes[i]; j++) {
                printf("Neuron %d: mp=%f, ts=%f\n", j, layers[i][j].mp, layers[i][j].ts);
            }
        }

        // Print all inputs
        printf("\nSnapshot of inputs at time %Lf\n", time);
        for (int i = 0; i < num_layers; i++) {
            printf("Layer %d\n", i);
            print_input_list(input_lists[i]);
            printf("\n");
        }

        printf("\nPerforming updates...\n");

        for (int layer_idx = 0; layer_idx < num_layers; layer_idx++) {
            INPUT* current = input_lists[layer_idx]->head;
            while (current != NULL) {

                int to_update_index = current->target_index;
                double to_update_value = current->value;

                LIFNeuron* neuron_to_update = &layers[layer_idx][to_update_index];

                double time_since_last_spike = time - neuron_to_update->ts;

                neuron_to_update->mp = RP + neuron_to_update->mp * (1.0 - exp(-time_since_last_spike)) + to_update_value;

                if (neuron_to_update->mp > ATH) {

                    neuron_to_update->ts = time;
                    neuron_to_update->mp = RP;


                    // If this is not the last layer, update the inputs to the next layer
                    if (layer_idx < num_layers - 1) {
                        for (int weight_idx = 0; weight_idx < layer_sizes[layer_idx + 1]; weight_idx++) {
                            double weight = weights[layer_idx][to_update_index][weight_idx];
                            if (weight > WTH) {
                                insert_input(input_lists[layer_idx + 1], weight_idx, weight);
                            }
                        }
                    } else {
                        output_buffer[to_update_index] = 1;
                    }

                }
                current = current->next; // move to the next input in the hash table
            }
            // Reset this layer's input list
            clear_input_list(input_lists[layer_idx]);
        }

        // Check the output buffer
        for (int i = 0; i < layer_sizes[num_layers - 1]; i++) {
            printf("Output %d: %d\n", i, output_buffer[i]);
            output_buffer[i] = 0; // Reset this output
        }

        // Advance time
        time += DT;
    }



    //////////////////////////////////////////// CLEANUP ////////////////////////////////////////////


    // Free layers list
    for (int i = 0; i < num_layers; i++) {
        free(layers[i]);
    }

    // Free input lists
    for (int i = 0; i < num_layers; i++) {
        free(input_lists[i]);
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
    free(input_lists);
    free(weights);
    
    //////////////////////////////////////////// END ////////////////////////////////////////////
    printf("\nDone!\n");
    return 0;
}


