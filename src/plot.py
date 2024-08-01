# This script is generated using chatGPT

import pandas as pd
import matplotlib.pyplot as plt

# Load data
data = pd.read_csv("bin/neuron_data.csv", header=None, names=["layer", "neuron", "time", "potential", "spike"])

# Define the number of neurons per layer
neurons_per_layer = [2, 3, 2]

# Calculate the total number of neurons
total_neurons = sum(neurons_per_layer)

# Create a figure with subplots for each neuron
fig, axes = plt.subplots(total_neurons, 1, figsize=(10, total_neurons * 2), sharex=True)

# Ensure axes is always iterable
if total_neurons == 1:
    axes = [axes]

current_row = 0

for layer, num_neurons in enumerate(neurons_per_layer):
    for neuron in range(num_neurons):
        neuron_data = data[(data["layer"] == layer) & (data["neuron"] == neuron)]
        ax = axes[current_row]
        
        # Plot spikes as red points
        spikes = neuron_data[neuron_data["spike"] == "spike"]
        ax.scatter(spikes["time"], spikes["potential"], color='red')
        
        ax.set_title(f"Layer {layer} - Neuron {neuron} Spikes")
        ax.set_ylabel("Membr. Pot. (mV)")
        ax.set_xlabel("Time (s)")
        ax.set_ylim(bottom=0)  # Ensure y-axis starts at 0
        
        current_row += 1

plt.xlabel("Time (s)")
plt.tight_layout()
plt.show()
