import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import seaborn as sns

import time
import busio
import board
import adafruit_amg88xx
i2c = busio.I2C(board.SCL1, board.SDA1)
amg = adafruit_amg88xx.AMG88XX(i2c)


# Function to update the heatmap for each frame
def update(frame):
    plt.clf()  # Clear the previous frame
    data = np.array(amg.pixels)
    data = np.flip(data, (0, 1))
    # Plot the heatmap
    # plt.imshow(data, cmap='coolwarm', vmin=0, vmax=50)
    sns.heatmap(data, annot=True, vmin=0, vmax=40, fmt='.1f', cmap='coolwarm')
    # plt.colorbar()

# Create a blank figure
fig, ax = plt.subplots()

# Create the animation object
ani = FuncAnimation(fig, update, frames=range(50), repeat=True)

# Display the animation
plt.show()

