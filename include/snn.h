#pragma once

#include <stdio.h> // For printf

#include<unistd.h> // For sleep

#include <math.h>
#include "../include/main.h"
#include "../include/network.h"

#ifndef VSCODE
#define STM32_PROFILE
#endif

#ifdef STM32_PROFILE
uint16_t UPDATE_LAYER_TOTAL_TIME = 0;
uint16_t UPDATE_INPUTS_0_TOTAL_TIME = 0;
uint16_t SET_INPUT_BUFFER_TOTAL_TIME = 0;
uint16_t UPDATE_MEMBRANE_POTENTIAL_TOTAL_TIME = 0;
#endif


#ifdef VSCODE
// If in VSCode
#include "../include/plotting.h"
#else
// If in STM32
// from tutorial https://youtu.be/sPzQ5CniWtw?si=RY3PwykBVtkKE4jr
int _write(int file, char *ptr, int len) {
	int i = 0;
	for (i = 0; i < len; i++) {
		ITM_SendChar((*ptr++));
	}
	return len;
}
#endif


// Function to get the absolute index of a neuron in the network
uint32_t absolute_neuron_index(int layer_idx, int neuron_idx) {
    return CNI[layer_idx] + neuron_idx;
} 

uint32_t absolute_weight_index(int layer_idx, int neuron_from_idx, int neuron_to_idx) {
    return CWI[layer_idx] + neuron_from_idx*LAYER_SIZES[layer_idx+1] + neuron_to_idx;
}

void set_input_buffer(int * num_input_spikes) {
    for (int neuron_idx = 0; neuron_idx < LAYER_SIZES[0]; neuron_idx+=1) {
        INPUT_BUFFER[neuron_idx] = 1;
        (*num_input_spikes)++;
    }
}

void update_inputs_0() {
    for (int input_idx = 0; input_idx < INPUT_SIZE; input_idx++) {

        if (INPUT_BUFFER[input_idx] == 1) {
            // If the current input is a spike, send it to all connected neurons in layer 0
            for (int next_neuron_idx = 0; next_neuron_idx < LAYER_SIZES[0]; next_neuron_idx++) {
                weight_t cur_weight = PRE_WEIGHTS[input_idx*LAYER_SIZES[0] + next_neuron_idx];
                if (cur_weight > WTH) {
                    uint32_t absolute_input_idx = absolute_neuron_index(0, next_neuron_idx);
                    if (INPUTS[absolute_input_idx] < ATH)
                        if (INPUTS[absolute_input_idx] + cur_weight < ATH) {
                            INPUTS[absolute_input_idx] += cur_weight;
                        } else {
                            INPUTS[absolute_input_idx] = ATH;
                        }
                    }
            }
            INPUT_BUFFER[input_idx] = 0; // Reset this input
        }
    }
}

neuron_mp_t update_membrane_potential(neuron_mp_t prev_mp, neuron_ts_t time_since_last_update) {
    return RP + (prev_mp - RP) * exp(- (long double) time_since_last_update / TAU);
}

void update_layer(int layer_idx, int * num_computations, unsigned long * time) {
    for (int cur_neuron_idx = 0; cur_neuron_idx < LAYER_SIZES[layer_idx]; cur_neuron_idx++) {
        
        int absolute_neuron_idx = absolute_neuron_index(layer_idx, cur_neuron_idx);

        if (INPUTS[absolute_neuron_idx] > WTH) {
            
            #ifdef MY_DEBUG
            printf("Updating neuron %d", cur_neuron_idx);
            #endif
            
            neuron_mp_t to_update_value = INPUTS[absolute_neuron_idx];

            neuron_mp_t neuron_to_update_mp = NEURONS_MP[absolute_neuron_idx];
            neuron_ts_t time_since_last_update = (*time) -                  NEURONS_TS[absolute_neuron_idx];

            #ifdef STM32_PROFILE
                uint16_t update_membrane_potential_time = __HAL_TIM_GET_COUNTER(&htim16);
            #endif
            neuron_to_update_mp = update_membrane_potential(neuron_to_update_mp, time_since_last_update);
            #ifdef PLOT
            fill_in_plot(layer_idx, cur_neuron_idx, NEURONS_TS[absolute_neuron_idx], *time, neuron_to_update_mp);
            mark_point(layer_idx, cur_neuron_idx, *time, neuron_to_update_mp);
            #endif
            neuron_to_update_mp += to_update_value;
            #ifdef PLOT
            mark_point(layer_idx, cur_neuron_idx, *time, neuron_to_update_mp);
            #endif
            #ifdef STM32_PROFILE
                UPDATE_MEMBRANE_POTENTIAL_TOTAL_TIME += __HAL_TIM_GET_COUNTER(&htim16) - update_membrane_potential_time;
                // printf("UPDATE_MEMBRANE_POTENTIAL_TOTAL_TIME: %d ms\n", UPDATE_MEMBRANE_POTENTIAL_TOTAL_TIME);
            #endif



            NEURONS_MP[absolute_neuron_idx] = neuron_to_update_mp;
            NEURONS_TS[absolute_neuron_idx] = (*time);

            // Check if the neuron spikes
            if (NEURONS_MP[absolute_neuron_idx] >= ATH) {
                
                NEURONS_MP[absolute_neuron_idx] = RP;
                #ifdef PLOT
                mark_point(layer_idx, cur_neuron_idx, *time, RP);
                #endif
                if (layer_idx < NUM_LAYERS - 1) {

                    #ifdef MY_DEBUG
                    printf(", spiked, connected to next layer indexes: ");
                    #endif

                    for (int next_neuron_idx = 0; next_neuron_idx < LAYER_SIZES[layer_idx+1]; next_neuron_idx++) {

                        int16_t cur_weight = WEIGHTS[absolute_weight_index(layer_idx, cur_neuron_idx, next_neuron_idx)];

                        if (cur_weight > WTH) {
                            #ifdef MY_DEBUG
                            printf("%d, ", next_neuron_idx); 
                            #endif
                            int absolute_next_input_idx = absolute_neuron_index(layer_idx+1, next_neuron_idx);
                            if (INPUTS[absolute_next_input_idx] < ATH) {
                                if (INPUTS[absolute_next_input_idx] + cur_weight < ATH) {
                                    INPUTS[absolute_next_input_idx] += cur_weight;
                                } else {
                                    INPUTS[absolute_next_input_idx] = ATH;
                                }
                            }
                        }
                    }
                    #ifdef MY_DEBUG
                    printf("\n");
                    #endif

                } else {
                    OUTPUT_BUFFER[cur_neuron_idx] = 1;
                    #ifdef MY_DEBUG
                    printf(", spiked, last layer\n");
                    #endif
                }
            } else {
                #ifdef MY_DEBUG
                printf("\n");
                #endif
            }

            (*num_computations)++; // Increment the number of computations
        }

        INPUTS[absolute_neuron_idx] = 0; // Reset the input for this neuron
    }
}


// The main function for the spiking neural network
void snn() {

    #ifdef PLOT
    // Record the layer dimensions to a csv file for plotting later
    write_layer_dimensions(LAYER_SIZES, NUM_LAYERS);
    add_headers();
    #endif

    // Initialize neuron membrande potentials
    for (int absolute_neuron_idx = 0; absolute_neuron_idx < NUM_NEURONS; absolute_neuron_idx++) {
        NEURONS_MP[absolute_neuron_idx] = RP;
    }


    
    // Print network
    #ifdef MY_DEBUG
    // Print all the weights
    printf("\n###### WEIGHTS\n");
    printf("\nInput -> Layer 0\n");
    for (int input_idx = 0; input_idx < INPUT_SIZE; input_idx++) {
        printf("[ ");
        for (int next_neuron_idx = 0; next_neuron_idx < LAYER_SIZES[0]; next_neuron_idx++) {
            // printf("Weight from input %d to neuron %d: %d\n", input_idx, next_neuron_idx, PRE_WEIGHTS[input_idx*LAYER_SIZES[0] + next_neuron_idx]);
            printf("%d ", PRE_WEIGHTS[input_idx*LAYER_SIZES[0] + next_neuron_idx]);
        }
        printf("]\n");
    }
    for (int layer_idx = 0; layer_idx < NUM_LAYERS-1; layer_idx++) {
        printf("\nLayer %d -> %d\n", layer_idx, layer_idx + 1);
        for (int neuron_idx = 0; neuron_idx < LAYER_SIZES[layer_idx]; neuron_idx++) {
            printf("[ ");
            for (int next_neuron_idx = 0; next_neuron_idx < LAYER_SIZES[layer_idx + 1]; next_neuron_idx++) {
                // printf("Weight from neuron %d to neuron %d: %d\n", neuron_idx, next_neuron_idx, WEIGHTS[layer_idx][neuron_idx][next_neuron_idx]);
                printf("%d ", WEIGHTS[absolute_weight_index(layer_idx, neuron_idx, next_neuron_idx)]);
            }
            printf("]\n");
        }
    }

    // Print all the neuron data
    printf("\n###### NEURONS\n");
    for (int layer_idx = 1; layer_idx < NUM_LAYERS; layer_idx++) {
        printf("\nLayer %d\n", layer_idx);
        for (int neuron_idx = 0; neuron_idx < LAYER_SIZES[layer_idx]; neuron_idx++) {
            printf("[ %d %d ]\n", NEURONS_MP[absolute_neuron_index(layer_idx, neuron_idx)], NEURONS_TS[absolute_neuron_index(layer_idx, neuron_idx)]);
        }
    }
    #endif


    #ifdef MY_DEBUG
    // Print the input queue for each layer
    printf("\n###### INPUTS\n");
    for (int layer_idx = 1; layer_idx < NUM_LAYERS; layer_idx++) {
        printf("\nLayer %d\n", layer_idx);
        for (int neuron_idx = 0; neuron_idx < LAYER_SIZES[layer_idx]; neuron_idx++) {
            printf("%d\n", INPUTS[absolute_neuron_index(layer_idx, neuron_idx)]);
        }
    }
    #endif
    
    
    ////////////////////////////////////// SETUP //////////////////////////////////////
    unsigned long time = 0.0;
    int num_iterations = 0;
    int num_computations = 0;
    int num_input_spikes = 0;

    #ifdef MY_DEBUG
    printf("\nStarting simulation with max number of computations: %d\n", MAX_COMPUTATIONS);
    #endif

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
    while (num_computations < MAX_COMPUTATIONS) {

        #ifdef MY_DEBUG
        printf("\n################################################## Iteration: %d\n", num_iterations);
        printf("\nNumber of computations: %d\n", num_computations);
        #endif

        // Start eatch iteration with sending spikes to every other neuron in the first layer
        #ifdef STM32_PROFILE
            uint16_t set_input_buffer_time = __HAL_TIM_GET_COUNTER(&htim16);
        #endif
        set_input_buffer(&num_input_spikes);
        #ifdef STM32_PROFILE
            SET_INPUT_BUFFER_TOTAL_TIME += __HAL_TIM_GET_COUNTER(&htim16) - set_input_buffer_time;
            // printf("SET_INPUT_BUFFER_TOTAL_TIME: %d ms\n", SET_INPUT_BUFFER_TOTAL_TIME);
        #endif
        
        // Send inputs to layer 1
        #ifdef STM32_PROFILE
            uint16_t update_inputs_0_time = __HAL_TIM_GET_COUNTER(&htim16);
        #endif
        update_inputs_0();
        #ifdef STM32_PROFILE
            UPDATE_INPUTS_0_TOTAL_TIME += __HAL_TIM_GET_COUNTER(&htim16) - update_inputs_0_time;
            // printf("UPDATE_INPUTS_0_TOTAL_TIME: %d ms\n", UPDATE_INPUTS_0_TOTAL_TIME);
        #endif
        
        // Go through layers from 1 to the last
        for (int layer_idx = 0; layer_idx < NUM_LAYERS; layer_idx++) {
            
            #ifdef MY_DEBUG
            printf("\n#### Layer %d\n", layer_idx);
            printf("\nInput:\n");
            printf("[ ");
            for (int i = 0; i < LAYER_SIZES[layer_idx]; i++) {
                printf("%d ", INPUTS[absolute_neuron_index(layer_idx, i)]);
            }
            printf("]\n");
            #endif

            #ifdef STM32_PROFILE
                uint16_t update_layer_time = __HAL_TIM_GET_COUNTER(&htim16);
            #endif
            update_layer(layer_idx, &num_computations, &time);
            #ifdef STM32_PROFILE
                UPDATE_LAYER_TOTAL_TIME += __HAL_TIM_GET_COUNTER(&htim16) - update_layer_time;
                // printf("UPDATE_LAYER_TOTAL_TIME: %d ms\n", UPDATE_LAYER_TOTAL_TIME);
            #endif
        }
        #ifdef MY_DEBUG
        printf("\n");

        // Check the output buffer
        for (int i = 0; i < LAYER_SIZES[NUM_LAYERS - 1]; i++) {
            if (OUTPUT_BUFFER[i] == 1) {
                printf("Neuron %d in the output layer spiked at time %ld\n", i, time);
            }
            OUTPUT_BUFFER[i] = 0; // Reset this output
        }
        #endif

        // Advance time and num_iterations
        time += DT;
        num_iterations++;
    }

    
    

    // Timer end
    double total_time_in_ms = 0.0;
    #ifndef VSCODE
    timer_val = __HAL_TIM_GET_COUNTER(&htim16) - timer_val;
    total_time_in_ms = ((double) timer_val);
    printf("\nUPDATE_LAYER_TOTAL_TIME: %d ms\n", UPDATE_LAYER_TOTAL_TIME);
    printf("\nUPDATE_MEMBRANE_POTENTIAL_TOTAL_TIME: %d ms\n", UPDATE_MEMBRANE_POTENTIAL_TOTAL_TIME);
    printf("\nSET_INPUT_BUFFER_TOTAL_TIME: %d ms\n", SET_INPUT_BUFFER_TOTAL_TIME);
    printf("\nUPDATE_INPUTS_0_TOTAL_TIME: %d ms\n", UPDATE_INPUTS_0_TOTAL_TIME);
    #else
    end = clock();
    total_time_in_ms = ((double) (end - start)) / CLOCKS_PER_SEC;
    #endif
    printf("\n################################################\n################################################\n");
    printf("\nTotal time: %f ms\n", total_time_in_ms);
    printf("\nInput size:    %d\n", INPUT_SIZE);
    printf("Network shape: [ ");
    for (int i = 0; i < NUM_LAYERS; i++) {
        printf("%d ", LAYER_SIZES[i]);
    };
    printf("]\n");
    printf("\nNumber of iterations: %d\n", num_iterations);
    printf("\nNumber of computations: %d\n", num_computations);


    
    //////////////////////////////////////////// END ////////////////////////////////////////////
    printf("\nDone with snn()!\n");

    
    
}