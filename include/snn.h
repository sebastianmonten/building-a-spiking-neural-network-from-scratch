#pragma once

#include <stdio.h> // For printf

#include<unistd.h> // For sleep

#include <math.h>
#include "../include/main.h"
#include "../include/network.h"

// #define STM32_PROFILE
#ifdef STM32_PROFILE
uint16_t UPDATE_LAYER_TOTAL_TIME = 0;
uint16_t UPDATE_INPUTS_1_TOTAL_TIME = 0;
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



void set_input_buffer(int * num_input_spikes) {
    for (int neuron_idx = 0; neuron_idx < LAYER_SIZES[0]-1; neuron_idx+=2) {
        INPUT_BUFFER[neuron_idx] = 1;
        (*num_input_spikes)++;
    }
}

void update_inputs_1() {
    for (int input_idx = 0; input_idx < LAYER_SIZES[0]; input_idx++) {
        if (INPUT_BUFFER[input_idx] == 1) {
            for (int weight_idx = 0; weight_idx < LAYER_SIZES[1]; weight_idx++) {
                weight_t cur_weight = WEIGHTS[0][input_idx][weight_idx];
                if (cur_weight > WTH && INPUTS[0][weight_idx] < ATH) {
                    INPUTS[0][weight_idx] += cur_weight;
                }
            }
            INPUT_BUFFER[input_idx] = 0; // Reset this input
        }
    }
}

decimal_temporary_t update_membrane_potential(decimal_temporary_t prev_mp, neuron_ts_t time_since_last_update) {
    return ((decimal_temporary_t) RP) + 
    (prev_mp - ((decimal_temporary_t ) RP)) * exp(- (long double) time_since_last_update / TAU);
}

void update_layer(int layer_idx, int * num_computations, unsigned long * time) {
    for (int input_idx = 0; input_idx < LAYER_SIZES[layer_idx]; input_idx++) {

        if (INPUTS[layer_idx-1][input_idx] > WTH) {
            
            #ifdef MY_DEBUG
            printf("Updating neuron %d", input_idx);
            #endif
            
            int to_update_index = input_idx;
            neuron_mp_t to_update_value = INPUTS[layer_idx-1][input_idx];

            decimal_temporary_t neuron_to_update_mp = (decimal_temporary_t) NEURONS_MP[layer_idx-1][to_update_index];
            neuron_ts_t time_since_last_update = (*time) -                  NEURONS_TS[layer_idx-1][to_update_index];

            // neuron_to_update_mp = 
            // ((decimal_temporary_t) RP) + 
            // (neuron_to_update_mp - ((decimal_temporary_t ) RP)) * exp(- (long double) time_since_last_update / TAU);
            #ifdef STM32_PROFILE
                uint16_t update_membrane_potential_time = __HAL_TIM_GET_COUNTER(&htim16);
            #endif
            neuron_to_update_mp = update_membrane_potential(neuron_to_update_mp, time_since_last_update);
            #ifdef STM32_PROFILE
                UPDATE_MEMBRANE_POTENTIAL_TOTAL_TIME += __HAL_TIM_GET_COUNTER(&htim16) - update_membrane_potential_time;
                // printf("UPDATE_MEMBRANE_POTENTIAL_TOTAL_TIME: %d ms\n", UPDATE_MEMBRANE_POTENTIAL_TOTAL_TIME);
            #endif

            neuron_to_update_mp += (decimal_temporary_t) to_update_value; 

            NEURONS_MP[layer_idx-1][to_update_index] = (neuron_mp_t) neuron_to_update_mp;
            NEURONS_TS[layer_idx-1][to_update_index] = (*time);

            if (NEURONS_MP[layer_idx-1][to_update_index] >= ATH) {
                
                NEURONS_MP[layer_idx-1][to_update_index] = RP;
                if (layer_idx < NUM_LAYERS - 1) {

                    #ifdef MY_DEBUG
                    printf(", spiked, connected to next layer indexes: ");
                    #endif

                    for (int weight_idx = 0; weight_idx < LAYER_SIZES[layer_idx+1]; weight_idx++) {
                        int16_t cur_weight = WEIGHTS[layer_idx][to_update_index][weight_idx];
                        if (cur_weight > WTH) {

                            #ifdef MY_DEBUG
                            printf("%d, ", weight_idx); 
                            #endif
                            if (INPUTS[layer_idx][weight_idx] < ATH) {
                                if ((uint32_t) INPUTS[layer_idx][weight_idx] + (uint32_t) cur_weight > (uint32_t) ATH) {
                                    #ifdef MY_DEBUG
                                    printf("(ATH reached), ");
                                    #endif
                                    INPUTS[layer_idx][weight_idx] = ATH;
                                } else {
                                    INPUTS[layer_idx][weight_idx] += cur_weight;
                                }
                            }
                        }
                    }
                    #ifdef MY_DEBUG
                    printf("\n");
                    #endif

                } else {
                    OUTPUT_BUFFER[to_update_index] = 1;
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
    }
}


// The main function for the spiking neural network
void snn() {

    // #ifdef PLOT
    // // Record the layer dimensions to a csv file for plotting later
    // write_layer_dimensions(LAYER_SIZES, NUM_LAYERS);
    // add_headers();
    // #endif

    // Initialize neuron membrande potentials
    for (int layer_idx = 1; layer_idx < NUM_LAYERS; layer_idx++) {
        for (int neuron_idx = 0; neuron_idx < LAYER_SIZES[layer_idx]; neuron_idx++) {
            NEURONS_MP[layer_idx-1][neuron_idx] = RP;
        }
    }


    
    // Print network
    #ifdef MY_DEBUG
    // Print all the weights
    printf("\n###### WEIGHTS\n");
    for (int layer_idx = 0; layer_idx < NUM_LAYERS-1; layer_idx++) {
        printf("\nLayer %d -> %d\n", layer_idx, layer_idx + 1);
        for (int neuron_idx = 0; neuron_idx < LAYER_SIZES[layer_idx]; neuron_idx++) {
            printf("[ ");
            for (int next_neuron_idx = 0; next_neuron_idx < LAYER_SIZES[layer_idx + 1]; next_neuron_idx++) {
                // printf("Weight from neuron %d to neuron %d: %d\n", neuron_idx, next_neuron_idx, WEIGHTS[layer_idx][neuron_idx][next_neuron_idx]);
                printf("%d ", WEIGHTS[layer_idx][neuron_idx][next_neuron_idx]);
            }
            printf("]\n");
        }
    }

    // Print all the neuron data
    printf("\n###### NEURONS\n");
    for (int layer_idx = 1; layer_idx < NUM_LAYERS; layer_idx++) {
        printf("\nLayer %d\n", layer_idx);
        for (int neuron_idx = 0; neuron_idx < LAYER_SIZES[layer_idx]; neuron_idx++) {
            printf("[ %d %d ]\n", NEURONS_MP[layer_idx-1][neuron_idx], NEURONS_TS[layer_idx-1][neuron_idx]);
        }
    }
    #endif


    #ifdef MY_DEBUG
    // Print the input queue for each layer
    printf("\n###### INPUTS\n");
    for (int layer_idx = 1; layer_idx < NUM_LAYERS; layer_idx++) {
        printf("\nLayer %d\n", layer_idx);
        for (int neuron_idx = 0; neuron_idx < LAYER_SIZES[layer_idx]; neuron_idx++) {
            printf("%d\n", INPUTS[layer_idx-1][neuron_idx]);
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
            uint16_t update_inputs_1_time = __HAL_TIM_GET_COUNTER(&htim16);
        #endif
        update_inputs_1();
        #ifdef STM32_PROFILE
            UPDATE_INPUTS_1_TOTAL_TIME += __HAL_TIM_GET_COUNTER(&htim16) - update_inputs_1_time;
            // printf("UPDATE_INPUTS_1_TOTAL_TIME: %d ms\n", UPDATE_INPUTS_1_TOTAL_TIME);
        #endif
        
        // Go through layers from 1 to the last
        for (int layer_idx = 1; layer_idx < NUM_LAYERS; layer_idx++) {
            
            #ifdef MY_DEBUG
            printf("\n#### Layer %d\n", layer_idx);
            printf("\nInput:\n");
            printf("[ ");
            for (int i = 0; i < LAYER_SIZES[layer_idx]; i++) {
                printf("%d ", INPUTS[layer_idx-1][i]);
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
    printf("\nUPDATE_INPUTS_1_TOTAL_TIME: %d ms\n", UPDATE_INPUTS_1_TOTAL_TIME);
    #else
    end = clock();
    total_time_in_ms = ((double) (end - start)) / CLOCKS_PER_SEC;
    #endif
    printf("\n################################################\n################################################\n");
    printf("\nTotal time: %f ms\n", total_time_in_ms);
    printf("\nNetwork shape: [");
    for (int i = 0; i < NUM_LAYERS; i++) {
        printf("%d ", LAYER_SIZES[i]);
    };
    printf("]\n");
    printf("\nNumber of iterations: %d\n", num_iterations);
    printf("\nNumber of computations: %d\n", num_computations);


    
    //////////////////////////////////////////// END ////////////////////////////////////////////
    printf("\nDone with snn()!\n");

    
    
}