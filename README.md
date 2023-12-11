# Occupancy Counting Based on IR Arrays

The project uses a neural network to classify human entrance and exist based on the IR arrays from the sensor AMG8833

## Contents

- [Recording](#recording)
- [Data Cleaning](#data-cleaning)
- [Arduino](#arduino)
- [Model](#model)

## Recording

### `record.py`

This script is responsible for recording the training dataset. To use this code, a Raspberry Pi Pico is required to connect the AMG8833 sensor to your local machine. For setup and usage instructions, refer to this detailed [tutorial](https://learn.adafruit.com/circuitpython-libraries-on-any-computer-with-raspberry-pi-pico/overview).

## Data Cleaning

### `data_cleaning.ipynb`

Jupyter Notebook containing the code for cleaning the dataset and performing data augmentation. This notebook outlines the steps taken to prepare the data for modeling.

### `random_forest.ipynb`

This Jupyter Notebook contains the code for feature extraction from raw data using a random forest algorithm.

## Arduino

For programming the Raspberry Pi Pico using the Arduino IDE, please follow the instructions provided by [arduino-pico](https://github.com/earlephilhower/arduino-pico). This will guide you through setting up your development environment.

### `Inference_3`

This folder contains the Arduino code for inference. It includes the main logic for processing and interpreting sensor data.

### `Inference_adafruit_mqtt`

This directory contains Arduino code for publishing inference results via MQTT using Adafruit IO. It's essential for remote monitoring and data transmission.

## Model

### `ei-human-entrance-final-arduino-1.0.3.zip`

This compressed file contains the machine learning model converted into an Arduino library format. The model is specifically tailored for the project's requirements and is ready for deployment on compatible hardware.

# occupancy_counting_AMG8833
Use Neural Network to detect human entrance and exit based on IR array
