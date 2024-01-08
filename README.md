
# SelfPlanter

SelfPlanter is an Arduino and Raspberry Pi-based project designed for an intelligent, self-sustaining plant cultivation experience. This system provides precise control over environmental parameters, ensuring optimal conditions for various plant species. The project incorporates robust sensors, smart watering mechanisms, and a live video stream with machine learning analysis for real-time plant health monitoring.

## Hardware Configuration

- **Arduino Mega:** Controls and monitors various sensors, relays, and pumps.
- **Raspberry Pi:** Streams live video and performs machine learning analysis.
- **DHT11 Sensor:** Temperature and humidity sensing.
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
const int LED_PIN = 13;
const int mq135Pin = A0;        
int soilPin = A1;               
int tempRelayPin = 11;          
int humRelayPin = 6;            
int airQualityRelayPin = 12;    
int soilMoistureRelayPin = 10;  

const int motor1Pin = 7;        
const int motor2Pin = 8;        
const int motor3Pin = 9;        
```

## Sensors and Actuators

- **Temperature and Humidity Sensor (DHT11):** Connected to pin 3.
- **MQ135 Gas Sensor:** Connected to analog pin A0.
- **Capacitive Soil Moisture Sensor:** Connected to analog pin A1.
- **Relays:** Used for controlling temperature, humidity, air quality, and soil moisture.
- **Watering Pumps (Motor Pins):** Connected to pins 7, 8, and 9.

## Setting Up

1. Connect the hardware components as per the defined pin configurations.
2. Install the necessary libraries mentioned in the Dependencies section.
3. Upload the provided Arduino sketch to the Arduino Mega.
4. Ensure the Raspberry Pi is set up with the required dependencies for video streaming and machine learning.

## Features

### Sensor Readings

- Reads temperature, humidity, air quality, and soil moisture from sensors.
- Displays sensor data every 2 seconds.

### LED Control

- The LED on pin 13 blinks based on the time of day.

### Manual Control

- Uses buttons to manually select plants from a predefined list.
- Activates corresponding relays and displays plant data on the OLED screen.

### Smart Watering

- Implements a water pump system based on nitrogen (N), phosphorus (P), and potassium (K) ratios.
- Calculates pump durations and displays the times on the OLED screen.

### Live Video Monitoring (Not Yet Integrated)

- Raspberry Pi streams live video of plants.
- Machine learning model analyzes the video for real-time plant health monitoring.

## Supported Plants (Not Yet Integrated)

- A predefined list of plants with growth parameters.

## Contribution

Contributions to the SelfPlanter project are welcome. Feel free to open issues, propose enhancements, or submit pull requests to improve the functionality and usability of this innovative plant cultivation system. Happy planting with SelfPlanter!

