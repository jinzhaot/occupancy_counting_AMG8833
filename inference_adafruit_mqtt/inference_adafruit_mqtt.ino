/* Edge Impulse ingestion SDK
 * Copyright (c) 2022 EdgeImpulse Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

/* Includes ---------------------------------------------------------------- */
#include <Wire.h>
#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <Adafruit_AMG88xx.h>
#include <human_entrance_final_inferencing.h>

Adafruit_AMG88xx amg;

const int numReadings = 10;
float sensorData[numReadings][AMG88xx_PIXEL_ARRAY_SIZE];
float summedData[640];
int keepingIndexList[] = {3,   4,   5,  10,  11,  12,  13,  18,  20,  21,  23,  25,  26,
        28,  29,  30,  31,  35,  36,  53,  54,  65,  67,  74,  75,  76,
        77,  81,  82,  83,  91,  92,  93,  99, 100, 114, 115, 116, 117,
       123, 124, 127, 131, 132, 138, 139, 140, 142, 145, 147, 148, 154,
       155, 157, 161, 164, 165, 172, 175, 176, 178, 179, 180, 186, 187,
       188, 194, 195, 196, 201, 203, 204, 207, 211, 212, 214, 217, 219,
       222, 227, 229, 232, 235, 243, 244, 245, 251, 252, 259, 260, 261,
       264, 267, 271, 274, 281, 282, 297, 299, 300, 303, 309, 312, 314,
       315, 323, 330, 332, 336, 338, 339, 341, 347, 362, 376, 378, 387,
       388, 389, 394, 395, 396, 403, 404, 405, 409, 413, 414, 427, 428,
       430, 431, 435, 436, 437, 438, 443, 444, 445, 450, 451, 452, 458,
       459, 460, 461, 466, 467, 468, 473, 476, 484, 486, 490, 491, 499,
       500, 501, 506, 507, 508, 509, 515, 516, 517, 521, 522, 524, 525,
       531, 532, 536, 539, 540, 547, 548, 555, 561, 563, 565, 568, 571,
       572, 576, 578, 580, 581, 586, 589, 594, 595, 596, 597, 601, 602,
       603, 604, 605, 610, 611, 613, 619, 639}; // important features
float features[sizeof(keepingIndexList) / sizeof(keepingIndexList[0])]; // Array to store important features

//WiFi parameters
#define WLAN_SSID       "xxx" // change to your wifi name
#define WLAN_PASS       "xxx" // change to your wifi password

// #define WLAN_SSID       "Ruirui&Zhaozhao's Home"
// #define WLAN_PASS       "tianloverui1314"

// Adafruit IO
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
//Enter the username and key from the Adafruit IO
#define AIO_USERNAME    "xxxx"  // change to your AIO_USERNAME
#define AIO_KEY         "xxxx"  // change to your AIO_KEY
WiFiClient client;
// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
// Adafruit_MQTT_Publish State = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/state");
Adafruit_MQTT_Publish State = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/state");

String state; //to store the inference result

/**
 * @brief      Copy raw feature data in out_ptr
 *             Function called by inference library
 *
 * @param[in]  offset   The offset
 * @param[in]  length   The length
 * @param      out_ptr  The out pointer
 *
 * @return     0
 */
int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
    memcpy(out_ptr, features + offset, length * sizeof(float));
    return 0;
}

void print_inference_result(ei_impulse_result_t result);

/**
 * @brief      Arduino setup function
 */
void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    Wire.begin();
    if (!amg.begin()) {
        Serial.println("Could not find a valid AMG88xx sensor, check wiring!");
        while (1);
    }
    delay(500); 

    Serial.print(F("Connecting to "));
    Serial.println(WLAN_SSID);
    WiFi.begin(WLAN_SSID, WLAN_PASS);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(F("."));
      
    }
    Serial.println();
    Serial.println(F("WiFi connected"));
    Serial.println(F("IP address: "));
    Serial.println(WiFi.localIP());

    // connect to adafruit io
    connect();
    delay(500); 
}

// connect to adafruit io via MQTT
void connect() {
  Serial.print(F("Connecting to Adafruit IO... "));
  int8_t ret;
  while ((ret = mqtt.connect()) != 0) {
    switch (ret) {
      case 1: Serial.println(F("Wrong protocol")); break;
      case 2: Serial.println(F("ID rejected")); break;
      case 3: Serial.println(F("Server unavail")); break;
      case 4: Serial.println(F("Bad user/pass")); break;
      case 5: Serial.println(F("Not authed")); break;
      case 6: Serial.println(F("Failed to subscribe")); break;
      default: Serial.println(F("Connection failed")); break;
    }

    if(ret >= 0)
      mqtt.disconnect();

    Serial.println(F("Retrying connection..."));
    delay(10000);
  }
  Serial.println(F("Adafruit IO Connected!"));
}


/**
 * @brief      Arduino main function
 */
void loop()
{
  // ping adafruit io a few times to make sure we remain connected
 
  if(! mqtt.ping(3)) {
    // reconnect to adafruit io
    if(! mqtt.connected())
      connect();
  }

  // read the sensor data and inferece
    for(int i = 0; i < numReadings; i++) {
        amg.readPixels(sensorData[i]);
        delay(80); // Delay between readings
    }

    normalizeData();
    selectData();

    delay(50); // Delay before starting next batch of readings

    Serial.println("finish data collection");


    ei_printf("Edge Impulse standalone inferencing (Arduino)\n");

    if (sizeof(features) / sizeof(float) != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
        ei_printf("The size of your 'features' array is not correct. Expected %lu items, but had %lu\n",
            EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, sizeof(features) / sizeof(float));
        delay(50);
        return;
    }

    ei_impulse_result_t result = { 0 };

    // the features are stored into flash, and we don't want to load everything into RAM
    signal_t features_signal;
    features_signal.total_length = sizeof(features) / sizeof(features[0]);
    features_signal.get_data = &raw_feature_get_data;

    // invoke the impulse
    EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false /* debug */);
    if (res != EI_IMPULSE_OK) {
        ei_printf("ERR: Failed to run classifier (%d)\n", res);
        return;
    }

    // print inference return code
    ei_printf("run_classifier returned: %d\r\n", res);
    print_inference_result(result);
    state = formatInferenceResult(result);

    Serial.println(state);

       if (! State.publish(state.c_str())) {                     //Publish to Adafruit
      Serial.println(F("Failed"));
    } 
    else {
      Serial.println(F("Sent!"));
    }
    delay(200);
}

void print_inference_result(ei_impulse_result_t result) {

    // Print how long it took to perform inference
    ei_printf("Timing: DSP %d ms, inference %d ms, anomaly %d ms\r\n",
            result.timing.dsp,
            result.timing.classification,
            result.timing.anomaly);

    ei_printf("Predictions:\r\n");
    for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        ei_printf("  %s: ", ei_classifier_inferencing_categories[i]);
        ei_printf("%.5f\r\n", result.classification[i].value);
    }
}

void normalizeData() {
    float globalMinVal = sensorData[0][0];
    float globalMaxVal = sensorData[0][0];

    // Find the global min and max values
    for (int i = 0; i < numReadings; i++) {
        for (int j = 0; j < AMG88xx_PIXEL_ARRAY_SIZE; j++) {
            if (sensorData[i][j] < globalMinVal) {
                globalMinVal = sensorData[i][j];
            }
            if (sensorData[i][j] > globalMaxVal) {
                globalMaxVal = sensorData[i][j];
            }
        }
    }
    const float scale = 1000.0; // Scale factor for 3 decimal places

    // Normalize the data in place based on the global min and max and round the data to 3 decimal places
    for (int i = 0; i < numReadings; i++) {
        for (int j = 0; j < AMG88xx_PIXEL_ARRAY_SIZE; j++) {
            sensorData[i][j] = (sensorData[i][j] - globalMinVal) / (globalMaxVal - globalMinVal + 1e-8);
            sensorData[i][j] = round(sensorData[i][j] * scale) / scale;
        }
    }
}


void selectData() {
    const int TOTAL_SIZE = numReadings * AMG88xx_PIXEL_ARRAY_SIZE;
    float flattenedSensorData[TOTAL_SIZE];

    // Flatten the sensorData array
    for (int i = 0; i < numReadings; i++) {
        for (int j = 0; j < AMG88xx_PIXEL_ARRAY_SIZE; j++) {
            flattenedSensorData[i * AMG88xx_PIXEL_ARRAY_SIZE + j] = sensorData[i][j];
        }
    }

    // Select data based on keepingIndexList
    for (int i = 0; i < sizeof(keepingIndexList) / sizeof(keepingIndexList[0]); i++) {
        int index = keepingIndexList[i];
        if (index < TOTAL_SIZE) {
            features[i] = flattenedSensorData[index];
        }
    }
}

String formatInferenceResult(ei_impulse_result_t result) {
    float max_value = 0.0;
    String max_category = "";

    // Find the category with the highest classification value
    for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        if (result.classification[i].value > max_value) {
            max_value = result.classification[i].value;
            max_category = ei_classifier_inferencing_categories[i];
        }
    }

    // Create a string that contains both max category and value
    String resultString = "Category: " + max_category + ", Probability: " + String(max_value);
    return resultString;
}


