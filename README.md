# SelfPlanter — embedded plant automation

Arduino Mega firmware for a plant-growing box that reads environmental sensors and controls watering, lighting, ventilation and nutrient pumps. This repository shows sensor integration, relay control, an OLED/button interface, EEPROM settings and RTC-based scheduling.

[Watch the hardware demo](https://www.youtube.com/shorts/UUdvqYnQaXs)

[![SelfPlanter demonstration](https://img.youtube.com/vi/UUdvqYnQaXs/hqdefault.jpg)](https://www.youtube.com/shorts/UUdvqYnQaXs)

## Firmware overview

- Plant presets and adjustable temperature, humidity, air-quality and soil-moisture thresholds.
- DHT22, MQ135 and soil-moisture sensor acquisition.
- Relay outputs for environmental control, watering and N/P/K dosing.
- SSD1306 OLED menu with three buttons.
- EEPROM persistence and DS3231 clock support.
- Serial telemetry and threshold updates for an external host.

The main implementation is [selfplanterV2.ino](selfplanterV2.ino). [lights.h](lights.h) and [AirPump.h](AirPump.h) contain lighting and air-pump logic. [SerialComm.py](SerialComm.py) is a host-side serial experiment; verify its framing against the firmware before using it.

## Hardware and pins

| Function | Arduino Mega pin |
| --- | --- |
| DHT22 | 3 |
| Light output | 13 |
| MQ135 / soil sensor | A13 / A14 |
| Temperature / humidity / air / soil relays | 12 / 11 / 10 / 9 |
| N / P / K pumps | 6 / 7 / 8 |
| Up / down / select buttons | 22 / 4 / 2 |
| OLED and DS3231 RTC | I²C |

The sketch configures a 128×64 SSD1306 display at address `0x3C`. Use appropriate drivers and separate supplies for pumps and other loads.

## Build and bring-up

1. Open `selfplanterV2.ino` in the Arduino IDE and select Arduino Mega 2560.
2. Install libraries matching the sketch includes: LibPrintf, MQ135, DHT, Adafruit GFX, Adafruit SSD1306 and RTClib. Wire, EEPROM and Arduino support come with the board package.
3. Check pin assignments, relay polarity and sensor calibration against your hardware.
4. Upload and open the serial monitor at **9600 baud**.
5. Set the clock and plant thresholds, then check sensor readings and each output individually before connecting the full system.

No library versions are pinned in this repository; compiling with your chosen versions and testing on hardware remain necessary.

## Serial interface

The firmware parses incoming threshold updates as a newline-terminated, hyphen-separated string:

```text
PlantName-temperature-humidity-airQuality-soilMoisture-N-P-K
Basil-20-50-500-60-3-1-2
```

Its telemetry uses a **different field order**:

```text
plantName-humidity-temperature-soilMoisture-airQuality-N-P-K
```

See `readThresholdValues()` and `printSensorData()` in the main sketch for the current format.

## Project scope

This is a hardware prototype. Thresholds, dosing and sensor-derived readings require calibration for the specific build. Camera-based plant-health analysis and machine learning are future work and are not implemented in the default firmware. The demo shows the physical project; it does not establish crop-yield or reliability measurements.
