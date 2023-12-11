import threading
from pynput import keyboard
import time
import numpy as np
import busio
import board
import adafruit_amg88xx
i2c = busio.I2C(board.SCL1, board.SDA1)
amg = adafruit_amg88xx.AMG88XX(i2c)


frames = []
recording = False

# Function to start recording
def start_recording():
    global frames
    global recording
    recording = True
    print("Recording...")

    while recording:
        data = np.array(amg.pixels)
        frames.append(data)
        time.sleep(0.1)

    print("Recording finished")

# Create a separate thread for recording
record_thread = threading.Thread(target=start_recording)

# Function to stop recording
def stop_recording():
    global frames
    global recording
    if recording:
        recording = False

        # Generate a unique filename based on the current timestamp
        current_time = time.strftime("%Y%m%d%H%M%S")
        filename = f"no_{current_time}.txt"

        # Save the recorded audio to the generated filename
        with open(filename, "w") as file:
            for array in frames:
                np.savetxt(file, array, fmt="%d", delimiter="\t")
                file.write("\n")
        print(f"saved as {filename}")
        frames = []

# Create a listener for the "space" key to stop recording
def on_key_release(key):
    if key == keyboard.Key.space:
        stop_recording()

# Create a listener for the "space" key to start recording
def on_key_press(key):
    global record_thread
    if key == keyboard.Key.space:
        if not record_thread.is_alive():
            record_thread = threading.Thread(target=start_recording)
            record_thread.start()

# Start the keyboard listeners
with keyboard.Listener(on_release=on_key_release, on_press=on_key_press) as listener:
    print("Press 'space' to start recording, and release 'space' to stop.")
    listener.join()