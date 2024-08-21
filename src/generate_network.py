# This script is created using chatGPT

import sys
import random
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors

MAX_VAL = 255
MIN_VAL = 8


def generate_weights(layer_sizes):
    weights = []
    num_layers = len(layer_sizes)

    # Ensure every neuron is connected to at least one neuron in the next layer
    for i in range(num_layers - 1):
        layer_weights = []
        for j in range(layer_sizes[i]):
            neuron_weights = [0] * layer_sizes[i + 1]
            k = random.randint(0, layer_sizes[i + 1] - 1)
            neuron_weights[k] = random.randint(MIN_VAL, MAX_VAL)  # Store as int16_t in range [100, 1000]
            layer_weights.append(neuron_weights)
        weights.append(layer_weights)

    # Ensure every neuron is connected to at least one neuron in the previous layer
    for i in range(1, num_layers):
        for k in range(layer_sizes[i]):
            j = random.randint(0, layer_sizes[i - 1] - 1)
            if weights[i - 1][j][k] == 0:
                weights[i - 1][j][k] = random.randint(MIN_VAL, MAX_VAL)

    # Randomly connect other neurons with some probability
    for i in range(num_layers - 1):
        for j in range(layer_sizes[i]):
            for k in range(layer_sizes[i + 1]):
                if random.random() > 0.75 and weights[i][j][k] == 0:  # 25% chance of creating a non-zero weight
                    weights[i][j][k] = random.randint(MIN_VAL, MAX_VAL)

    return weights

def save_weights_to_header(weights, layer_sizes, filename="include/network.h"):
    with open(filename, 'w') as f:

        
        num_layers = len(layer_sizes)
        num_weights = sum([layer_sizes[i] * layer_sizes[i + 1] for i in range(num_layers - 1)])
        num_neurons = sum(layer_sizes[1:])

        f.write('// This file is generated by "src/generate_network.py", called with "make network"\n')
        f.write("#pragma once\n\n")
        f.write('#include "../include/main.h"\n\n')

        f.write(f"const neuron_idx_t INPUT_SIZE = {layer_sizes[0]};\n")
        f.write("const neuron_idx_t LAYER_SIZES[] = {")
        for i in range(1, len(layer_sizes)):
            f.write(f"{layer_sizes[i]}")
            if i != len(layer_sizes) - 1:
                f.write(", ")
        f.write("};\n")
        f.write(f'const uint16_t NUM_LAYERS = {len(layer_sizes)-1};\n')

        f.write(f'const uint32_t NUM_WEIGHTS = {num_weights};\n')
        f.write(f'const uint32_t NUM_NEURONS = {num_neurons};\n')

        space = "    "

        # Writing weights for the first layer
        f.write('\nconst weight_t PRE_WEIGHTS[] = {\n')
        for j in range(layer_sizes[0]):
            f.write(space+f'/*neuron {j}*/ ')
            for k in range(layer_sizes[1]):
                weight_value = weights[0][j][k]
                f.write(f"{weight_value}, ")
            f.write("\n")
        f.write("};\n")

        # Writing weights for the rest of the layers
        f.write("\nconst weight_t WEIGHTS[] = {\n")
        for i in range(1, num_layers - 1):
            f.write(space + f"// {i-1} -> {i}\n")
            # f.write(space + "(const weight_t*[]){\n")
            
            for j in range(layer_sizes[i]):
                f.write(space+f'/*neuron {j}*/ ')
                for k in range(layer_sizes[i + 1]):
                    weight_value = weights[i][j][k]
                    f.write(f"{weight_value}, ")
                f.write("\n")
            f.write("\n")
        f.write("};\n")

        f.write("\n// Cumulative weight index sum for each layer\n")
        f.write("uint32_t CWI[] = {")
        c_sum = 0
        for i in range(1, len(layer_sizes)-1):
            f.write(f"{c_sum}, ")
            c_sum += layer_sizes[i]*layer_sizes[i+1]
        f.write("};\n")

        f.write("\n// Cumulative neuron index sum for each layer\n")
        f.write("uint16_t CNI[] = {")
        c_sum = 0
        for i in range(1, len(layer_sizes)):
            f.write(f"{c_sum}, ")
            c_sum += layer_sizes[i]
        f.write("};\n")

        f.write("\n// Neuron membrane potentials\n")
        f.write("neuron_mp_t NEURONS_MP[] = {\n")
        for i in range(1, len(layer_sizes)):
            f.write(space)
            for j in range(layer_sizes[i]):
                f.write("0, ")
            f.write("\n")
        f.write("};\n")

        f.write("\n// Timestamps for when neurons where last updated\n")
        f.write("neuron_ts_t NEURONS_TS[] = {\n")
        for i in range(1, len(layer_sizes)):
            f.write(space)
            for j in range(layer_sizes[i]):
                f.write("0, ")
            f.write("\n")
        f.write("};\n")

        f.write("\n// Buffer slots for accumuled inputs for each neuron from spikes from the previous layer\n")
        f.write("input_t INPUTS[] = {\n")
        for i in range(1, len(layer_sizes)):
            f.write(space)
            for j in range(layer_sizes[i]):
                f.write("0, ")
            f.write("\n")
        f.write("};\n")

        f.write(f"\n// Boolen array representing spikes from the {layer_sizes[0]} inputs\n")
        f.write("input_t INPUT_BUFFER[] = {")
        for i in range(layer_sizes[0]):
            f.write("0, ")
        f.write("};\n")

        f.write(f"\n// Boolen array representing spikes from the {layer_sizes[-1]} neurons in the last layer\n")
        f.write("input_t OUTPUT_BUFFER[] = {")
        for i in range(layer_sizes[-1]):
            f.write("0, ")
        f.write("};\n")
    


def plot_network(layer_sizes, weights):
    fig, ax = plt.subplots(figsize=(12, 8))
    num_layers = len(layer_sizes)

    # Calculate positions for each neuron
    layer_positions = []
    max_neurons = max(layer_sizes)
    for i, layer_size in enumerate(layer_sizes):
        x_pos = i
        # y_pos = np.linspace(-max_neurons / 2, max_neurons / 2, layer_size)
        y_pos = np.linspace(max_neurons / 2, -max_neurons / 2, layer_size)
        layer_positions.append((x_pos, y_pos))

    # Plot neurons with larger dots
    for i, (x, y) in enumerate(layer_positions):
        ax.scatter([x] * len(y), y, s=300, label=f'Layer {i + 1}')  # Increased 's' to 300

    # Plot connections with color based on weight value
    norm = mcolors.Normalize(vmin=0, vmax=MAX_VAL)
    cmap = plt.get_cmap('Reds')

    for i, layer in enumerate(weights):
        for j, neuron in enumerate(layer):
            for k, weight in enumerate(neuron):
                if weight != 0:
                    color = cmap(norm(weight))
                    ax.plot([layer_positions[i][0], layer_positions[i + 1][0]],
                            [layer_positions[i][1][j], layer_positions[i + 1][1][k]],
                            color=color, lw=1.5)

    ax.axis('off')
    plt.savefig('tmp/network_visualization.png')
    print("Plot saved as 'tmp/network_visualization.png'")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 generate_weights.py [layer_sizes]")
        sys.exit(1)

    layer_sizes = eval(sys.argv[1])

    # Generate weights
    weights = generate_weights(layer_sizes)

    # Save weights to a header file
    save_weights_to_header(weights, layer_sizes)
    print(f"Network data saved to 'include/network.h'")

    # Calculate the total number of neurons
    total_neurons = sum(layer_sizes)

    # Conditionally plot the network
    if total_neurons <= 100:
        plot_network(layer_sizes, weights)
        # # Print the weights for MY_DEBUGging
        # for i, layer in enumerate(weights):
        #     for j, neuron in enumerate(layer):
        #         for k, weight in enumerate(neuron):
        #             if weight != 0:
        #                 print(f"Weight from neuron {j} in layer {i} to neuron {k} in layer {i + 1}: {weight}")
    else:
        print(f"Total number of neurons ({total_neurons}) exceeds 100, skipping visualization.")
