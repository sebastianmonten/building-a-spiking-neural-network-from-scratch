#pragma once

#include <stdio.h> // For printf

#include <math.h>
#include "../include/main.h"
#include "../include/input_queue.h"
#include "../include/weights.h"



#ifdef VSCODE
#include "../include/plotting.h"
#endif

neuron_idx_t get_absolute_neuron_idx(neuron_idx_t layer_idx, neuron_idx_t neuron_idx) {
    neuron_idx_t absolute_neuron_idx = 0;
    for (int i = 0; i < layer_idx; i++) {
        absolute_neuron_idx += LAYER_SIZES[i];
    }
    return absolute_neuron_idx + neuron_idx;
}


// from layer 1, neuron 1 to 0
neuron_idx_t get_absolute_weight_idx(neuron_idx_t from_layer_idx, neuron_idx_t from_neuron_idx, neuron_idx_t to_neuron_idx) {
    neuron_idx_t absolute_weight_idx = 0;
    for (int i = 0; i < from_layer_idx; i++) {
        absolute_weight_idx += LAYER_SIZES[i] * LAYER_SIZES[i + 1];
    }
    return absolute_weight_idx + from_neuron_idx * LAYER_SIZES[from_layer_idx + 1] + to_neuron_idx;
}

// The main function for the spiking neural network
void snn() {


    // printf("\nComparison of getting weight index:\n");
    // int from_layer = 1;
    // int from_neuron = 1;
    // int to_neuron = 0;
    // printf("Target weight index A: %d\n", from_layer * LAYER_SIZES[from_layer] * LAYER_SIZES[from_layer + 1] + from_neuron * LAYER_SIZES[from_layer + 1] + to_neuron);
    // printf("Target weight index B: %d\n", get_absolute_weight_idx(from_layer, from_neuron, to_neuron));


    #ifdef VSCODE
    // Record the layer dimensions to a csv file for plotting later
    write_layer_dimensions(LAYER_SIZES, NUM_LAYERS);
    add_headers();
    #endif

    // Allocate memory for the array of layers
    NEURONS_MP = (neuron_mp_t*)malloc(NUM_NEURONS * sizeof(neuron_mp_t));
    NEURONS_TS = (neuron_ts_t*)malloc(NUM_NEURONS * sizeof(neuron_ts_t));
    
    // Define lower and upper bounds
    int lower_bound = 100;
    int upper_bound = 1000;

    // Print all layer dimensions and weights
    #ifdef MY_DEBUG
    printf("\nLayer sizes: ");
    for (int i = 0; i < NUM_LAYERS; i++) {
        printf("%d ",LAYER_SIZES[i]);
    }
    printf("\n");

    printf("\nWeights:\n");
    for (int from_layer_idx = 0; from_layer_idx < NUM_LAYERS - 1; from_layer_idx++) {
        printf("From layer %d to %d\n", from_layer_idx, from_layer_idx + 1);
        for (int from_neuron_idx = 0; from_neuron_idx < LAYER_SIZES[from_layer_idx]; from_neuron_idx++) {
            printf("  ");
            for (int to_neuron_idx = 0; to_neuron_idx < LAYER_SIZES[from_layer_idx + 1]; to_neuron_idx++) {
                printf("%d ", WEIGHTS[get_absolute_weight_idx(from_layer_idx, from_neuron_idx, to_neuron_idx)]);
            }
            printf("\n");
        }
    }
    #endif


    // Allocate memory for and initialize neurons. Print all neurons
    #ifdef MY_DEBUG
    printf("\nNeurons:\n");
    #endif
    for (int layer_idx = 0; layer_idx < NUM_LAYERS; layer_idx++) {
        #ifdef MY_DEBUG
        printf("Layer %d\n", layer_idx);
        #endif
        for (int neuron_idx = 0; neuron_idx < LAYER_SIZES[layer_idx]; neuron_idx++) {
            int absolute_neuron_idx = get_absolute_neuron_idx(layer_idx, neuron_idx);
            NEURONS_MP[absolute_neuron_idx] = RP;
            NEURONS_TS[absolute_neuron_idx] = 0;

            #ifdef MY_DEBUG
            printf("[%d %d] ", NEURONS_MP[absolute_neuron_idx], NEURONS_TS[absolute_neuron_idx]);
            #endif
        }
        #ifdef MY_DEBUG
        printf("\n\n");
        #endif
    }

    

    // Create a queue of input queues for each layer
    INPUT_QUEUE** input_queues = (INPUT_QUEUE**)malloc(NUM_LAYERS * sizeof(INPUT_QUEUE*));
    for (uint16_t layer_idx = 0; layer_idx < NUM_LAYERS; layer_idx++) {
        input_queues[layer_idx] = create_input_queue(LAYER_SIZES[layer_idx]);
    }

    #ifdef MY_DEBUG
    // Print the input queue for each layer
    printf("Snapshot of input queues:\n");
    for (uint16_t layer_idx = 0; layer_idx < NUM_LAYERS; layer_idx++) {
        printf("Layer %d\n", layer_idx);
        print_input_queue(input_queues[layer_idx]);
        printf("\n");
    }
    #endif

    
    ////////////////////////////////////// SETUP //////////////////////////////////////
    unsigned long time = 0.0;
    int num_iterations = 0;
    int num_computations = 0;
    int num_input_spikes = 0;


    // Set up output buffer
    int output_buffer[LAYER_SIZES[NUM_LAYERS - 1]];
    for (int i = 0; i < LAYER_SIZES[NUM_LAYERS - 1]; i++) {
        output_buffer[i] = 0;
    }

    // Timer setup
    #ifdef VSCODE
    // IF VSCODE
    clock_t start, end;
    start = clock();
    #else
    // IF STM32
    HAL_TIM_Base_Start(&htim16);
    uint16_t timer_val = __HAL_TIM_GET_COUNTER(&htim16);
    #endif

    
    // Main Loop
    printf("\nStarting simulation with max computations: %d\n", MAX_COMPUTATIONS);
    while (num_computations < MAX_COMPUTATIONS) {

        
        printf("\n################################################## Iteration: %d\n", num_iterations);
        


        // Start eatch iteration with sending spikes to every other neuron in the first layer
        for (int layer_idx = 0; layer_idx < LAYER_SIZES[0]-1; layer_idx+=2) {
            insert_input(input_queues[0], layer_idx, 256);
            num_input_spikes++;
        }
        

        // Forward pass
        for (int layer_idx = 0; layer_idx < NUM_LAYERS; layer_idx++) {
            
            #ifdef MY_DEBUG
            // Print a snapshot of the current input queue
            printf("Snapshot of input queue for layer %d\n", layer_idx);
            print_input_queue(input_queues[layer_idx]);

            printf("\n");

            // Print a snapshot of the current membrane potentials
            printf("\nSnapshot of membrane potentials for layer %d\n", layer_idx);
            for (int neuron_idx = 0; neuron_idx < LAYER_SIZES[layer_idx]; neuron_idx++) {
                printf(
                    "[%d, %d] ",
                    NEURONS_MP[get_absolute_neuron_idx(layer_idx, neuron_idx)],
                    NEURONS_TS[get_absolute_neuron_idx(layer_idx, neuron_idx)]
                );
            }

            printf("\n\n");
            #endif

            // Check if the current layer has any inputs to take care of
            INPUT* current = input_queues[layer_idx]->head;
            if (current == NULL) {
                #ifdef MY_DEBUG
                printf("No inputs to layer %d, end this iteration early\n", layer_idx);
                #endif
                break; // If current layer has no input, then nor will the rest. Break the loop
            }

            // Go through all update tasks in the current layer's queue
            while (current != NULL) {

                if (num_computations >= MAX_COMPUTATIONS) {
                    break;
                }

                #ifdef MY_DEBUG
                printf("\nUpdating neuron %d in layer %d at time %ld\n", current->target_index, layer_idx, time);
                #endif

                uint16_t to_update_index = current->target_index; // The index of the neuron to update
                neuron_mp_t to_update_value = current->value; // The input value to the neuron

                // LIFNeuron* neuron_to_update = &layers[layer_idx][to_update_index]; // Pointer to neuron to update
                // float time_since_last_update = time - neuron_to_update->ts;
                uint16_t neuron_to_update_absolute_idx = get_absolute_neuron_idx(layer_idx, to_update_index);
                decimal_temporary_t neuron_to_update_mp = (decimal_temporary_t) NEURONS_MP[neuron_to_update_absolute_idx];
                neuron_ts_t time_since_last_update = time - NEURONS_TS[neuron_to_update_absolute_idx];
                
                #ifdef VSCODE
                // Add datapoints for the membrane potential between the last update and now
                fill_in_plot(
                    layer_idx,
                    to_update_index,
                    NEURONS_TS[neuron_to_update_absolute_idx],
                    time,
                    NEURONS_MP[neuron_to_update_absolute_idx]
                );
                #endif

                // Calculate the new membrane potential after it has been leaking since the last update
                neuron_to_update_mp = 
                    ((decimal_temporary_t) RP) + 
                    (neuron_to_update_mp - ((decimal_temporary_t ) RP)) * exp(- (long double) time_since_last_update / TAU);
                // Add the input and update the time stamp
                neuron_to_update_mp += (decimal_temporary_t) to_update_value; 
                NEURONS_MP[neuron_to_update_absolute_idx] = (neuron_mp_t) neuron_to_update_mp; // Conert back and update the membrane potential
                NEURONS_TS[neuron_to_update_absolute_idx] = time;

                #ifdef VSCODE
                // Plot the increace in membrane potential as a new point at the same time as the last data point
                mark_point(layer_idx,
                    to_update_index,
                    time,
                    NEURONS_MP[neuron_to_update_absolute_idx]
                );
                #endif
                
                // Check for spike
                if (NEURONS_MP[neuron_to_update_absolute_idx] > ATH) {
                    
                    #ifdef MY_DEBUG
                    printf("Neuron %d in layer %d spiked at time %ld\n", to_update_index, layer_idx, time);
                    #endif

                    NEURONS_MP[neuron_to_update_absolute_idx] = RP; // Reset the membrane potential

                    #ifdef VSCODE
                    mark_point(
                        layer_idx, 
                        to_update_index,
                        time, 
                        NEURONS_MP[neuron_to_update_absolute_idx]
                    ); // Mark the spike as a vertical line down to RP
                    #endif

                    // If this is not the last layer, update the inputs to the next layer
                    #ifdef MY_DEBUG
                    printf("\nIt has the followintg connections to the next layer:\n");
                    #endif
                    if (layer_idx < NUM_LAYERS - 1) {
                        for (int weight_idx = 0; weight_idx < LAYER_SIZES[layer_idx+1]; weight_idx++) {
                            int16_t cur_weight = WEIGHTS[ get_absolute_weight_idx(layer_idx, to_update_index, weight_idx) ];
                            #ifdef MY_DEBUG
                            printf("%d ", cur_weight);
                            #endif
                            if (cur_weight > WTH) {
                                // float converted_weight = convert_weight(weight, lower_bound, upper_bound);
                                insert_input(input_queues[layer_idx + 1], weight_idx, cur_weight);
                            }
                        }
                        #ifdef MY_DEBUG
                        printf("\n\n");
                        #endif
                    } else {
                        output_buffer[to_update_index] = 1;
                    }
                }
                current = current->next; // Move to the next input in the queue
                num_computations++; // Increment the number of computations
            }
            // Reset this layer's input queue
            clear_input_queue(input_queues[layer_idx]);
        }

        // Check the output buffer
        for (int i = 0; i < LAYER_SIZES[NUM_LAYERS - 1]; i++) {
            if (output_buffer[i] == 1) {
                // printf("Neuron %d in the output layer spiked at time %Lf\n", i, time);
            }
            output_buffer[i] = 0; // Reset this output
        }

        // Advance time and num_iterations
        time += DT;
        num_iterations++;
    }


    // Timer end
    printf("\n##############################################################\n##############################################################\n");
    #ifndef VSCODE
    timer_val = __HAL_TIM_GET_COUNTER(&htim16) - timer_val;
    printf("%d neuron computations\r\n", num_computations);
    printf("%d input spikes\r\n", num_input_spikes);
    printf("%f s\r\n", ((double) timer_val) / 1000);
    printf("Done!\n");
    #else
    end = clock();
    printf("\nNumber of computations: %d\n", num_computations);
    printf("\nNumber of input spikes: %d\n", num_input_spikes);
    printf("\nNumber of iterations: %d\n", num_iterations);
    printf("\nTime taken: %f seconds\n", ((double) (end - start)) / CLOCKS_PER_SEC);
    #endif



    //////////////////////////////////////////// CLEANUP ////////////////////////////////////////////


    // Free layer sizes and input queues
    for (int layer_idx = 0; layer_idx < NUM_LAYERS; layer_idx++) {
        free(input_queues[layer_idx]);
    }
    free(input_queues);
    free(NEURONS_MP);
    // free(NEURONS_TS);
    
    //////////////////////////////////////////// END ////////////////////////////////////////////
    printf("\nDone with snn()!\n");

    
    
}