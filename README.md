
# SelfPlanter

SelfPlanter is an Arduino and Raspberry Pi-based project designed for an intelligent, self-sustaining plant cultivation experience. This system provides precise control over environmental parameters, ensuring optimal conditions for various plant species. The project incorporates sensors such as temperature, humidity, Co2, SoilMoisture, smart watering mechanisms, and a live video stream with machine learning analysis for real-time plant health monitoring.

# How did this project come to be

The self-planter came about because I was tired of failing at growing plants like strawberries, lettuce, and rosemary due to the wrong weather conditions. I wanted something that could handle all the important stuff like temperature, humidity, soil moisture, and fertilization for me. Now, I've got this nifty system where I just pop in the seed and hit a button. No need to keep checking on it – it takes care of everything on its own.

# Whats the difference between other irrigation systems 

- **Effort and Practicality:** I've dedicated a lot of time and effort to ensure that every aspect of this project serves a practical purpose. Each feature is designed with functionality in mind, aiming to mimic natural growth mechanisms as closely as possible.

- **Sunlight Simulation:** The lighting system operates on a schedule managed by an RTC (Real-Time Clock). At sunrise, the lights gradually fade in over a 15-minute period, replicating the gradual increase in sunlight. Similarly, at sunset, the lights fade out over 15 minutes to simulate dusk.

- **Watering Mechanism:** The watering system is meticulously designed to maintain optimal soil moisture levels. It allows the soil to dry out slightly more than usual before watering and slightly overwaters to ensure a healthy range of moisture levels, mirroring natural rainfall patterns. This is achieved through a pipe embedded in the soil with evenly distributed tiny holes, simulating rainfall.

- **Nutrient Management:** NPK ratios are automatically mixed into the water, eliminating the hassle of manually mixing fertilizers into the soil. Water-based fertilizer is utilized for this purpose, ensuring convenient and effective nutrient delivery to the plants.

- **Water Runout Safety:** An automatic water runout mechanism prevents overwatering by shutting off the pump if it runs for an extended period without a change in soil moisture levels, indicating that the water supply is depleted.

- **State Memory:** The system utilizes the EEPROM of the Arduino to remember previous settings and actions, such as previously set values and whether the water was fertilized or not, ensuring continuity and convenience in operation.

- **Modular Design:** Adding new plants is straightforward. Through the serial port, users can easily input the necessary values in the specified format, allowing for flexibility and customization in plant selection and care.

- **Compatibility:** The project is adaptable to both Raspberry Pi and standalone operation. The Raspberry Pi provides additional functionalities such as live video streaming and internet access, but the system is designed in a modular manner, requiring minimal adjustments to the code for either configuration.

  
## Hardware Configuration

- **Arduino Mega:** Controls and monitors various sensors, relays, and pumps.
- **Raspberry Pi:** Streams live video and performs API fetching.
- **DHT22 Sensor:** Temperature and humidity sensing.
- **MQ135 Gas Sensor:** Monitors air quality.
- **Capacitive Soil Moisture Sensor:** Ensures reliable soil moisture readings.

## Dependencies

- [LibPrintf](https://github.com/embeddedartistry/arduino-libprintf): Library for advanced printf functionality.
- [MQ135 Library](https://github.com/GeorgK/MQ135): Library for interfacing with the MQ135 gas sensor.
- [DHT Library](https://github.com/adafruit/DHT-sensor-library): Library for interfacing with the DHT11 sensor.
- [Wire Library](https://www.arduino.cc/en/Reference/Wire): Arduino library for I2C communication.
- [Adafruit SSD1306 Library](https://github.com/adafruit/Adafruit_SSD1306): Library for interfacing with SSD1306 OLED displays.
- [RTClib](https://github.com/adafruit/RTClib): Library for interfacing with the DS3231 real-time clock.

## Pin Configuration

```cpp
// all pins will be defined here
#define DHTPIN 3
#define LIGHTPIN 13
const int mq135Pin = A13;           // Analog input pin connected to the MQ135 gas sensorww
const int soilPin = A14;            // soil moisture sensor pin
const int tempRelayPin = 12;        // temperature relay pin
const int humRelayPin = 11;         // humidity relay pin
const int airQualityRelayPin = 10;  // air quality relay pin
const int soilMoistureRelayPin = 9; // soil moisture relay pin
const int motorN = 6;               // Pin connected to the motor Nitrogen pump
const int motorP = 7;               // Pin connected to the motor Phosphorus pump
const int motorK = 8;               // Pin connected to the motor Potasium pump
#define BUTTON_UP 22
#define BUTTON_DOWN 4
#define BUTTON_SELECT 2
```

## Sensors and Actuators

- **Temperature and Humidity Sensor (DHT22):** Connected to pin 3.
- **MQ135 Gas Sensor:** Connected to analog pin A13.
- **Capacitive Soil Moisture Sensor:** Connected to analog pin A14.
- **Relays:** Used for controlling temperature, humidity, air quality, and soil moisture.
- **Watering Pumps (Motor Pins):** Connected to pins 6, 7, and 8.

## Setting Up

1. Connect the hardware components as per the defined pin configurations.
2. Install the necessary libraries mentioned in the Dependencies section.
3. Upload the provided Arduino sketch to the Arduino Mega.
4. Ensure the Raspberry Pi is set up with the required dependencies for video streaming and machine learning.

## Features

### Sensor Readings

- Reads temperature, humidity, air quality, and soil moisture from sensors.
- Displays sensor data every second.

### Light Control

- The LED on pin 13 fades in and out based on the time of day, replicating natural sunlight.

### sensor control

- Uses buttons to manually select plants from a predefined list.
- Activates corresponding relays and displays plant data on the OLED screen.
- The system also uses the serial port if you want to import your own plant, it takes the data as follows through the serial monitor and saves it in the eeprom.
-Input and output format
PlantName-humidity-temperature-soilMoisture-airquality-N-P-K
Example:
Strawberry-60-70-18.00-400-10-20-30
- The pi takes this data and sends it to the api which then sends it to the website or the data can be monitored globally.

### Smart Watering

- Implements a water pump which is fertillised through a system for nitrogen (N), phosphorus (P), and potassium (K) ratios.
- Calculates pump durations and displays the times on the OLED screen.

### Live Video Monitoring (Not Yet Integrated)

- Raspberry Pi streams live video of plants.
- Machine learning model analyzes the video for real-time plant health monitoring.

## Supported Plants (Not Yet Integrated)
```cpp
Plant plants[] = {
    {"Strawberry", 60, 70, 18.0, 400, "10-20-30"},
    {"Basil", 50, 60, 20.0, 500, "3-1-2"},
    {"Iceberg Lettuce", 60, 70, 16.0, 500, "10-10-10"},
    {"Mint", 50, 60, 22.0, 450, "6-3-3"},
    {"Spinach", 60, 70, 15.0, 500, "5-10-10"},
    {"Rosemary", 40, 50, 18.0, 400, "10-6-8"},
    {"Thyme", 50, 60, 18.0, 450, "5-10-10"},
    {"Cilantro", 50, 60, 22.0, 500, "4-1-2"},
    {"Chives", 50, 60, 18.0, 450, "5-10-5"},
    {"Parsley", 50, 60, 20.0, 500, "5-10-10"},
    {"Sage", 40, 50, 18.0, 400, "10-8-6"},
    {"Lemon Balm", 50, 60, 22.0, 450, "5-10-5"},
    {"Oregano", 40, 50, 18.0, 400, "10-8-6"},
    {"Lavender", 40, 50, 18.0, 400, "5-10-10"},
    {"Dill", 50, 60, 22.0, 500, "4-6-4"},
    {"Celery", 60, 70, 15.0, 500, "5-10-10"},
    {"Kale", 60, 70, 15.0, 500, "4-2-6"},
    {"Arugula", 50, 60, 18.0, 500, "5-10-5"},
    {"Cabbage", 60, 70, 15.0, 500, "5-10-10"},
    {"Germination", 70, 60, 30.0, 500, "0-0-0"}};
```

## Contribution

Contributions to the SelfPlanter project are welcome. Feel free to open issues, propose enhancements, or submit pull requests to improve the functionality and usability of this innovative plant cultivation system. Happy planting with SelfPlanter!

