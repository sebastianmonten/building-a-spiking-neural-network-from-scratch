# This script is created using chatGPT

import sys
import random
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors


include_str = '#include "../include/main.h"\n\n'

def generate_weights(layer_sizes):
    weights = []
    num_layers = len(layer_sizes)

    # Ensure every neuron is connected to at least one neuron in the next layer
    for i in range(num_layers - 1):
        layer_weights = []
        for j in range(layer_sizes[i]):
            neuron_weights = [0] * layer_sizes[i + 1]
            k = random.randint(0, layer_sizes[i + 1] - 1)
            neuron_weights[k] = random.randint(100, 1000)  # Store as int16_t in range [100, 1000]
            layer_weights.append(neuron_weights)
        weights.append(layer_weights)

    # Ensure every neuron is connected to at least one neuron in the previous layer
    for i in range(1, num_layers):
        for k in range(layer_sizes[i]):
            j = random.randint(0, layer_sizes[i - 1] - 1)
            if weights[i - 1][j][k] == 0:
                weights[i - 1][j][k] = random.randint(100, 1000)

    # Randomly connect other neurons with some probability
    for i in range(num_layers - 1):
        for j in range(layer_sizes[i]):
            for k in range(layer_sizes[i + 1]):
                if random.random() > 0.75 and weights[i][j][k] == 0:  # 25% chance of creating a non-zero weight
                    weights[i][j][k] = random.randint(100, 1000)

    return weights

def save_weights_to_header(weights, layer_sizes, filename="include/weights.h"):
    with open(filename, 'w') as f:
        f.write("#pragma once\n")
        f.write(include_str)
        
        num_layers = len(layer_sizes)
        # Writing weights in the specified format
        for i in range(num_layers - 1):
            for j in range(layer_sizes[i]):
                for k in range(layer_sizes[i + 1]):
                    weight_value = weights[i][j][k]
                    if weight_value != 0:
                        f.write(f"weights[{i}][{j}][{k}] = {weight_value};\n")


def plot_network(layer_sizes, weights):
    fig, ax = plt.subplots(figsize=(12, 8))
    num_layers = len(layer_sizes)

    # Calculate positions for each neuron
    layer_positions = []
    max_neurons = max(layer_sizes)
    for i, layer_size in enumerate(layer_sizes):
        x_pos = i
        y_pos = np.linspace(-max_neurons / 2, max_neurons / 2, layer_size)
        layer_positions.append((x_pos, y_pos))

    # Plot neurons with larger dots
    for i, (x, y) in enumerate(layer_positions):
        ax.scatter([x] * len(y), y, s=300, label=f'Layer {i + 1}')  # Increased 's' to 300

    # Plot connections with color based on weight value
    norm = mcolors.Normalize(vmin=100, vmax=1000)
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
    print(f"Weights saved to 'include/weights.h'")

    # Calculate the total number of neurons
    total_neurons = sum(layer_sizes)

    # Conditionally plot the network
    if total_neurons <= 100:
        plot_network(layer_sizes, weights)
        # Print the weights for MY_DEBUGging
        for i, layer in enumerate(weights):
            for j, neuron in enumerate(layer):
                for k, weight in enumerate(neuron):
                    if weight != 0:
                        print(f"Weight from neuron {j} in layer {i} to neuron {k} in layer {i + 1}: {weight}")
    else:
        print(f"Total number of neurons ({total_neurons}) exceeds 100, skipping visualization.")
