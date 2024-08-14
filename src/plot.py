# This script is generated using chatGPT

import pandas as pd
import matplotlib.pyplot as plt
import itertools
import matplotlib

# Use the Agg backend
matplotlib.use('Agg')

# Define a limit for neurons and layers
MAX_NEURONS_PER_LAYER = 10
MAX_LAYERS = 6

# Read the layer dimensions CSV file
layer_dimensions = pd.read_csv('tmp/layer_dimensions.csv')

# Ensure the layer dimensions do not exceed the limits
if len(layer_dimensions) > MAX_LAYERS:
    raise ValueError(f"Number of layers exceeds the maximum allowed ({MAX_LAYERS}).")

if any(layer_dimensions['neurons'] > MAX_NEURONS_PER_LAYER):
    raise ValueError(f"Number of neurons in a layer exceeds the maximum allowed ({MAX_NEURONS_PER_LAYER}).")

# Read the neuron data CSV file
data = pd.read_csv('tmp/neuron_data.csv')

# Define distinct colors and line styles for the neurons
colors = ['r', 'g', 'b', 'c', 'm', 'y', 'k', 'orange', 'purple', 'brown']
line_styles = ['-', '--', '-.', ':']
color_line_combinations = list(itertools.product(colors, line_styles))

# Create a figure and subplots
fig, axes = plt.subplots(nrows=len(layer_dimensions), ncols=1, sharex=True, figsize=(10, 8))

# Plot data for each layer
for layer in range(len(layer_dimensions)):
    num_neurons = layer_dimensions.iloc[layer]['neurons']
    layer_data = data[data['layer'] == layer]
    for neuron in range(num_neurons):
        neuron_data = layer_data[layer_data['neuron'] == neuron]
        color, line_style = color_line_combinations[neuron % len(color_line_combinations)]
        axes[layer].plot(neuron_data['time'], neuron_data['potential'], label=f'Neuron {neuron}', color=color, linestyle=line_style)
    
    axes[layer].set_title(f'Layer {layer}')
    axes[layer].set_ylabel('Potential')
    axes[layer].legend()
    axes[layer].set_ylim(0, 300)  # Set y-axis limits

# Set x-axis label for the last subplot
axes[-1].set_xlabel('Time')

# Adjust layout
plt.tight_layout()

# Save the plot or display
plt.savefig('tmp/neuron_potentials.png')
# plt.show() // Does not seem to work in wsl environment