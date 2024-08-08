# This script is generated using chatGPT

import sys
import random
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors

def generate_weights(layer_sizes):
    weights_output = []
    num_layers = len(layer_sizes)

    # Ensure every neuron is connected to at least one neuron in the next layer
    for i in range(num_layers - 1):
        for j in range(layer_sizes[i]):
            k = random.randint(0, layer_sizes[i + 1] - 1)
            weight_value = random.uniform(0.1, 1.0)
            weights_output.append(f'weights[{i}][{j}][{k}] = {weight_value};')

    # Ensure every neuron is connected to at least one neuron in the previous layer
    for i in range(1, num_layers):
        for k in range(layer_sizes[i]):
            j = random.randint(0, layer_sizes[i - 1] - 1)
            weight_value = random.uniform(0.1, 1.0)
            weights_output.append(f'weights[{i - 1}][{j}][{k}] = {weight_value};')

    # Randomly connect other neurons with some probability
    for i in range(num_layers - 1):
        for j in range(layer_sizes[i]):
            for k in range(layer_sizes[i + 1]):
                if random.random() > 0.75:  # 25% chance of creating a non-zero weight
                    weight_value = random.uniform(0.1, 1.0)
                    weights_output.append(f'weights[{i}][{j}][{k}] = {weight_value};')

    return weights_output

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

    # Plot neurons
    for i, (x, y) in enumerate(layer_positions):
        ax.scatter([x] * len(y), y, s=200, label=f'Layer {i+1}')

    # Plot connections with color based on weight value
    norm = mcolors.Normalize(vmin=0.1, vmax=1.0)
    cmap = plt.get_cmap('Reds')

    for i, layer in enumerate(weights):
        for j, neuron in enumerate(layer):
            for k, weight in enumerate(neuron):
                if weight != 0.0:
                    color = cmap(norm(weight))
                    ax.plot([layer_positions[i][0], layer_positions[i+1][0]],
                            [layer_positions[i][1][j], layer_positions[i+1][1][k]],
                            color=color, lw=1.5)

    ax.axis('off')
    plt.savefig('tmp/network_visualization.png')
    print("\nPlot saved as 'tmp/network_visualization.png'")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 generate_weights.py [layer_sizes]")
        sys.exit(1)

    layer_sizes = eval(sys.argv[1])

    # Generate and print weights
    weights_output = generate_weights(layer_sizes)
    for line in weights_output:
        print(line)

    # Convert weights to a 3D list for visualization
    num_layers = len(layer_sizes)
    weights = [[[0.0 for _ in range(layer_sizes[i + 1])] for _ in range(layer_sizes[i])]
               for i in range(num_layers - 1)]

    for line in weights_output:
        exec(line)  # Executes the string as code to set the weights

    # Plot the network
    plot_network(layer_sizes, weights)
