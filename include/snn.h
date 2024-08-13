#pragma once

#include <stdio.h> // For printf


#include "../include/main.h"
#include "../include/input_queue.h"
#include "../include/LIF_neuron.h"
#include "../include/load_weights_from_binary.h"


#ifndef STM32CubeIDE
#include "../include/plotting.h"
#else
#undef DEBUG
#endif



void snn() {


    // Define the number of neurons in each layer
    int layer_sizes[] = {16384, 3, 2}; // Number of neurons in each layer
    int num_layers = sizeof(layer_sizes) / sizeof(layer_sizes[0]);


    #ifndef STM32CubeIDE
    // Record the layer dimensions to a csv file for plotting later
    write_layer_dimensions(layer_sizes, num_layers);
    add_headers();
    #endif

    // Allocate memory for the array of layers
    LIFNeuron** layers = (LIFNeuron**)malloc(num_layers * sizeof(LIFNeuron*));

    // Allocate memory for the array of weight matrices
    weights = (double***)malloc((num_layers - 1) * sizeof(double**)); // Already defined in weights.h

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
    
    ///////////////////////////////// setting the weights ///////////////////////
    // Load weights from the binary file to the weights array
    load_weights_from_binary(weights, layer_sizes, num_layers, "bin/weights.bin");
    ////////////////////////////////////////////////////////////////////////////

    // print all weights
    #ifdef DEBUG
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
    #endif


    // Allocate memory for and initialize neurons
    for (int i = 0; i < num_layers; i++) {
        layers[i] = (LIFNeuron*)malloc(layer_sizes[i] * sizeof(LIFNeuron));
        for (int j = 0; j < layer_sizes[i]; j++) {
            layers[i][j].mp = RP;
            layers[i][j].ts = 0.0;
        }
    }

    // Print all neurons
    #ifdef DEBUG
    for (int i = 0; i < num_layers; i++) {
        printf("\nLayer %d neurons:\n", i);
        for (int j = 0; j < layer_sizes[i]; j++) {
            printf("Neuron %d: mp=%f, ts=%f\n", j, layers[i][j].mp, layers[i][j].ts);
        }
    }
    #endif

    // Create a queue of input queues for each layer
    INPUT_QUEUE** input_queues = (INPUT_QUEUE**)malloc(num_layers * sizeof(INPUT_QUEUE*));
    for (int i = 0; i < num_layers; i++) {
        input_queues[i] = create_input_queue(layer_sizes[i]);
    }
    
    
    // Print the input queue for each layer
    #ifdef DEBUG
    printf("\nInput queues for each layer\n");
    for (int i = 0; i < num_layers; i++) {
        printf("Layer %d\n", i);
        print_input_queue(input_queues[i]);
        printf("\n");
    }
    #endif

    ////////////////////////////////////// SETUP //////////////////////////////////////
    long double time = 0.0;
    int counter = 0;
    int num_computations = 0;
    int num_input_spikes = 0;

    


    


    // Set up output buffer
    int output_buffer[layer_sizes[num_layers - 1]];
    for (int i = 0; i < layer_sizes[num_layers - 1]; i++) {
        output_buffer[i] = 0;
    }

    // Timer setup
    #ifdef STM32CubeIDE
    HAL_TIM_Base_Start(&htim16);
    timer_val = __HAL_TIM_GET_COUNTER(&htim16);
    #else
    clock_t start, end;
    start = clock();
    #endif


    #ifdef DEBUG
    printf("\nMax computations: %d\n", MAX_COMPUTATIONS);
    printf("\nStarting simulation\n");
    #endif

    // Main Loop
    while (num_computations < MAX_COMPUTATIONS) {

        
        #ifdef DEBUG
        printf("\n################################################## Iteraion: %d\n", counter);
        #endif

        if (num_computations % 50 == 0) {
            printf("Iteration: %d, Computations: %d, Input spikes: %d\n", counter, num_computations, num_input_spikes);
        }

        // Start eatch iteration with sending spikes to all neurons in the first layer
        for (int i = 0; i < layer_sizes[0]; i++) {
            #ifdef DEBUG
            printf("Inserting input to neuron %d in the input layer\n", i);
            #endif
            insert_input(input_queues[0], i, 1.0);
            num_input_spikes++;
        }

        // Forward pass
        for (int layer_idx = 0; layer_idx < num_layers; layer_idx++) {

            #ifdef DEBUG
            // print a snaphot of neuron states:
            printf("\nNeuron states at layer %d:\n", layer_idx);
            for (int i = 0; i < layer_sizes[layer_idx]; i++) {
                printf("Neuron %d: mp=%f, ts=%f\n", i, layers[layer_idx][i].mp, layers[layer_idx][i].ts);
            }

            printf("\nInputs to layer %d:\n", layer_idx);
            print_input_queue(input_queues[layer_idx]);
            #endif

            INPUT* current = input_queues[layer_idx]->head;
            if (current == NULL) {
                #ifdef DEBUG
                printf("No inputs to layer %d\n", layer_idx);
                #endif
                break; // If current layer has no input, then nor will the rest. Break the loop
            }





            while (current != NULL) {

                if (num_computations >= MAX_COMPUTATIONS) {
                    break;
                }

                #ifdef DEBUG
                printf("\nUpdating neuron %d in layer %d\n", current->target_index, layer_idx);
                #endif

                int to_update_index = current->target_index; // The index of the neuron to update
                double to_update_value = current->value; // The input value to the neuron

                LIFNeuron* neuron_to_update = &layers[layer_idx][to_update_index]; // Pointer to neuron to update
                double time_since_last_update = time - neuron_to_update->ts;
                
                #ifndef STM32CubeIDE
                // Add datapoints for the membrane potential between the last update and now
                fill_in_plot(layer_idx, to_update_index, neuron_to_update->ts, time, neuron_to_update->mp);
                #endif

                // Calculate the new membrane potential after it has been leaking since the last update
                neuron_to_update->mp = RP + (neuron_to_update->mp - RP) * exp(-time_since_last_update / TAU);
                // Add the input and update the time stamp
                neuron_to_update->mp += to_update_value; 
                neuron_to_update->ts = time;

                #ifndef STM32CubeIDE
                // Plot the increace in membrane potential as a new point at the same time as the last data point
                mark_point(layer_idx, to_update_index, time, neuron_to_update->mp);
                #endif
                
                // Check for spike
                if (neuron_to_update->mp > ATH) {

                    neuron_to_update->mp = RP; // Reset the membrane potential

                    #ifndef STM32CubeIDE
                    mark_point(layer_idx, to_update_index, time, neuron_to_update->mp); // Mark the spike as a vertical line down to RP
                    #endif

                    #ifdef DEBUG
                    printf("Neuron %d in layer %d spiked at time %Lf\n", to_update_index, layer_idx, time);
                    #endif

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
                num_computations++; // Increment the number of computations
            }
            // Reset this layer's input queue
            clear_input_queue(input_queues[layer_idx]);
        }

        // Check the output buffer
        for (int i = 0; i < layer_sizes[num_layers - 1]; i++) {
            if (output_buffer[i] == 1) {
                // printf("Neuron %d in the output layer spiked at time %Lf\n", i, time);
            }
            output_buffer[i] = 0; // Reset this output
        }

        // Advance time and counter
        time += DT;
        counter++;
    }


    // Timer end
    #ifdef STM32CubeIDE
    timer_val = __HAL_TIM_GET_COUNTER(&htim16) - timer_val;
    printf("%d neuron computations\r\n", num_computations);
    printf("%d input spikes\r\n", num_input_spikes);
    printf("%u ms\r\n", timer_val);
    printf("Done!\n");
    #else
    end = clock();
    printf("\nNumber of computations: %d\n", num_computations);
    printf("\nNumber of input spikes: %d\n", num_input_spikes);
    printf("\nNumber of iterations: %d\n", counter);
    printf("\nTime taken: %f seconds\n", ((double) (end - start)) / CLOCKS_PER_SEC);
    #endif



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
    
}