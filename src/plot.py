# This script is created with ChatGPT

import pandas as pd
import matplotlib.pyplot as plt

# Load the data
file_path = "bin/tmp_output.csv"
data = pd.read_csv(file_path)

# Create subplots with shared x-axis
fig, axs = plt.subplots(4, 1, sharex=True, figsize=(10, 8))

# Plot each MemPot
axs[0].plot(data['Time'], data['MemPot0'])
axs[0].set_ylabel('MemPot 0')

axs[1].plot(data['Time'], data['MemPot1'])
axs[1].set_ylabel('MemPot 1')

axs[2].plot(data['Time'], data['MemPot2'])
axs[2].set_ylabel('MemPot 2')

axs[3].plot(data['Time'], data['MemPot3'])
axs[3].set_ylabel('MemPot 3')
axs[3].set_xlabel('Time')

# Adjust layout
plt.tight_layout()
plt.show()
