#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

typedef struct INPUT {
    int index;
    double value;
    struct INPUT* next;
} INPUT;

typedef struct {
    INPUT** table;
    int size;
} HashTable;

// Hash function
unsigned int hash(int index, int size) {
    return index % size;
}

// Create a new input
INPUT* create_input(int index, double value) {
    INPUT* new_input = (INPUT*)malloc(sizeof(INPUT));
    new_input->index = index;
    new_input->value = value;
    new_input->next = NULL;
    return new_input;
}

// Initialize the hash table
HashTable* create_table(int size) {
    HashTable* hashTable = (HashTable*)malloc(sizeof(HashTable));
    hashTable->size = size;
    hashTable->table = (INPUT**)malloc(sizeof(INPUT*) * size);
    for (int i = 0; i < size; i++) {
        hashTable->table[i] = NULL;
    }
    return hashTable;
}

// Insert or update an input
void insert(HashTable* hashTable, int index, double value) {
    unsigned int hashIndex = hash(index, hashTable->size);
    INPUT* current = hashTable->table[hashIndex];
    
    // Check if index already exists and update value
    while (current != NULL) {
        if (current->index == index) {
            current->value += value;
            return;
        }
        current = current->next;
    }

    // If index does not exist, create a new entry
    INPUT* new_input = create_input(index, value);
    new_input->next = hashTable->table[hashIndex];
    hashTable->table[hashIndex] = new_input;
}

// Reset the value of an input
void reset(HashTable* hashTable, int index) {
    unsigned int hashIndex = hash(index, hashTable->size);
    INPUT* current = hashTable->table[hashIndex];
    
    // Check if index exists and reset value
    while (current != NULL) {
        if (current->index == index) {
            current->value = 0.0;
            return;
        }
        current = current->next;
    }
}

// Iterate over all elements
void iterate(HashTable* hashTable) {
    for (int i = 0; i < hashTable->size; i++) {
        INPUT* current = hashTable->table[i];
        while (current != NULL) {
            printf("Index: %d, Value: %f\n", current->index, current->value);
            current = current->next;
        }
    }
}

// Free the hash table
void free_table(HashTable* hashTable) {
    for (int i = 0; i < hashTable->size; i++) {
        INPUT* current = hashTable->table[i];
        while (current != NULL) {
            INPUT* to_free = current;
            current = current->next;
            free(to_free);
        }
    }
    free(hashTable->table);
    free(hashTable);
}






//////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
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
    // HashTable* hashTable = create_table(10);
    
    // insert(hashTable, 1, 10.5);
    // insert(hashTable, 2, 20.5);
    // insert(hashTable, 1, 5.0);
    // insert(hashTable, 15, 30.5);  // This will collide with index 1 if size is 10

    // iterate(hashTable);
    
    // free_table(hashTable);




    // Define the number of neurons in each layer
    int layer_sizes[] = {2, 3, 2};
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

    // Create array of hash tables, one for each neuron layer, including the input layer
    HashTable** inputs = (HashTable**)malloc(num_layers * sizeof(HashTable*));
    // initialize all hash to be empty
    for (int i = 0; i < num_layers; i++) {
        inputs[i] = create_table(layer_sizes[i]);
    }


    // Setup
    long double time = 0.0;
    int output_spike = 0;
    // insert inputs to first inputs hash table
    insert(inputs[0], 0, 1.0);
    insert(inputs[0], 1, 1.0);

    // Main Loop
    printf("\nStarting simulation...\n");
    while (time < 10.0) {
        printf("\n###########################################\nTime: %Lf\n", time);

        // introduce input after 5 seconds
        if (time == 5.0) {
            insert(inputs[0], 0, 1.0);
            insert(inputs[0], 1, 1.0);
        }

        // Print all neurons
        for (int i = 0; i < num_layers; i++) {
            printf("\nLayer %d neurons:\n", i);
            for (int j = 0; j < layer_sizes[i]; j++) {
                printf("Neuron %d: mp=%f, ts=%f\n", j, layers[i][j].mp, layers[i][j].ts);
            }
        }

        for (int layer_idx = num_layers - 1; layer_idx > -1; layer_idx--) {
            
            INPUT* current = inputs[layer_idx]->table[layer_idx];

            // iterate through the inputs to the layer layer_idx
            while (current != NULL) {

                int to_update_index = current->index;
                double to_update_value = current->value;

                LIFNeuron* neuron_to_update = &layers[layer_idx][to_update_index];

                double time_since_last_spike = time - neuron_to_update->ts;
                neuron_to_update->mp = RP + neuron_to_update->mp * (1.0 - exp(-time_since_last_spike)) + to_update_value;

                if (neuron_to_update->mp > ATH) {
                    printf("Neuron %d in layer %d spiked at time %Lf\n", to_update_index, layer_idx);

                    neuron_to_update->ts = time;
                    neuron_to_update->mp = RP;

                    // If this is not the last layer, update the inputs to the next layer
                    if (layer_idx < num_layers - 1) {
                        for (int weight_idx = 0; weight_idx < layer_sizes[layer_idx + 1]; weight_idx++) {
                            double weight = weights[layer_idx][to_update_index][weight_idx];
                            if (weight > WTH) {
                                // this is where the program breaks
                                insert(inputs[layer_idx + 1], weight_idx, weight);
                            }
                        }
                    } else {
                        output_spike = 1;
                    }

                }

                reset(inputs[layer_idx], to_update_index); // reset the input after updating the neuron
                
                current = current->next; // move to the next input in the hash table
            }
            
        }
        // printf("\n");
        time += DT;
    }
    


    printf("\nDone!\n");
    return 0;
}
