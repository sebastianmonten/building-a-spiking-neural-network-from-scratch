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
const double time_tolerance = 0.0001; // tolerance for time comparisons
//////////////////////////////////////////////////////////////////////


////////////////////// LIFNeuron struct //////////////////////////////
typedef struct {
    double mp; // membrane potential
    double ts; // time-stamp of last spike
} LIFNeuron;
//////////////////////////////////////////////////////////////////////



///////////////////////////// PLOTTING //////////////////////////////////
void mark_point(int layer, int neuron, double time, double potential) {
    FILE *file = fopen("tmp/neuron_data.csv", "a");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    // If the file is empty, add headers
    fseek(file, 0, SEEK_END);
    // Check if the file is empty
    if (ftell(file) == 0) {
        // Add headers to the CSV file
        fprintf(file, "layer,neuron,time,potential\n");
    }

    // Make an entry in the CSV file
    fprintf(file, "%d,%d,%f,%f\n", layer, neuron, time, potential);
    fclose(file);
}
//////////////////////////////////////////////////////////////////////



int main() {

    // Define the number of neurons in each layer
    int layer_sizes[] = {2, 2, 1}; // {16, 20, 20, 100, 20, 4};
    int num_layers = sizeof(layer_sizes) / sizeof(layer_sizes[0]);


    // Record the layer dimensions
    FILE *file = fopen("tmp/layer_dimensions.csv", "a");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }
    fprintf(file, "neurons\n");
    for (int i = 0; i < num_layers; i++) {
        fprintf(file, "%d\n", layer_sizes[i]);
    }
    fclose(file);

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
    weights[0][0][1] = 0.2;
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

                int to_update_index = current->target_index;
                double to_update_value = current->value;

                LIFNeuron* neuron_to_update = &layers[layer_idx][to_update_index];

                // Go though the time steps and update the membrane potential
                double time_i = neuron_to_update->ts;
                while(time_i < time - time_tolerance) {
                    time_i += DT;
                    neuron_to_update-> mp += -(neuron_to_update->mp - RP) * DT / TAU;
                    mark_point(layer_idx, to_update_index, time_i, neuron_to_update->mp);
                    
                }
                // Add the new input to the current mp
                neuron_to_update->mp += to_update_value;
                mark_point(layer_idx, to_update_index, time_i, neuron_to_update->mp);

                // Update the time-stamp of the last update
                neuron_to_update->ts = time;

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


