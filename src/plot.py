# This script is created with ChatGPT

import numpy as np
import matplotlib.pyplot as plt

# Read the CSV file into NumPy arrays
data = np.genfromtxt('bin/tmp_output.csv', delimiter=',', skip_header=1)

# Separate the columns
time = data[:, 0]
mem_pot_1 = data[:, 1]
mem_pot_2 = data[:, 2]

# Create a figure and two subplots
fig, (ax1, ax2) = plt.subplots(2, 1, sharex=True)

# Plot MemPot 1
ax1.plot(time, mem_pot_1, label='MemPot 1')
ax1.set_ylabel('MemPot 1')
ax1.legend()

# Plot MemPot 2
ax2.plot(time, mem_pot_2, label='MemPot 2', color='orange')
ax2.set_ylabel('MemPot 2')
ax2.set_xlabel('Time')
ax2.legend()

# Show the plot
plt.show()
