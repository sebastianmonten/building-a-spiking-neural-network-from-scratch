import pandas as pd
import matplotlib.pyplot as plt

# Read the CSV file
df = pd.read_csv('bin/tmp_output.csv')

# Plot the data
plt.plot(df['Time'], df['Membrane Potential'])
plt.xlabel('Time (s)')
plt.ylabel('Membrane Potential (V)')
plt.title('Membrane Potential over Time')
plt.grid(True)
plt.show()