#include "LibPrintf.h"
#include "MQ135.h"
#include "DHT.h"
#include <Wire.h>
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include "RTClib.h"
#include "lights.h"
#include "AirPump.h"

// DEFINE YOUR PINS HERE
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

const int eepromSize = 30; // Set the EEPROM size as needed

#define DHTTYPE DHT22
#define RLOAD 22.0

// Declare the sensor objects
DHT dht(DHTPIN, DHTTYPE);
Lights light(LIGHTPIN, 5);
MQ135 gasSensor = MQ135(mq135Pin);
AirPump pump(motorN, motorP, motorK);
RTC_DS3231 rtc;

// variables read from the sensor
float hum = 0;
int soilMoisture = 0;
int soilMoistureRaw;
float temp = 0;
float ppm; // decleration of variable for the air quality in ppm

// Variables to store time
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;

unsigned long startTimeforpump = 0; // the time when the sensor value first fell below the threshold
bool functionCalled = false;        // flag to indicate if the function has been called

int tempThreshold;         // temperature threshold (in °C) generally 25, stored in eeprom address 5
int humThreshold;          // humidity threshold (in %) genrally 40, stored in eeprom address 10
int airQualityThreshold;   // air quality threshold generally 1800, stored in eeprom address 15
int soilMoistureThreshold; // soil moisture threshold generally 10, stored in eeprom address 20
String plantName;          // store in address 40 onwards
int plantSelected;         // sored in EEPROM address 2
int previouslyFertilized;  // stored in EEPROM address 0

int N; // store in EEPROM address 25
int P; // store in EEPROM address 30
int K; // store in EEPROM address 35

const float pumpRate = 5.0 / 1000.0; // Pump rate in liters per second
const float totalSolution = 100.0;   // Total solution in liters

// Oled display parameters
#define OLED_RESET A3
// Adafruit_SSD1306 display(OLED_RESET);
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);
#define BUTTON_UP 22
#define BUTTON_DOWN 4
#define BUTTON_SELECT 2

// put all this in the raspberry pi, all plant data
struct Plant
{
  const char *name;
  uint8_t humidity;
  uint8_t soil_moisture;
  float temperature;
  uint16_t co2_level;
  const char *npk_ratio;
};

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

const uint8_t num_plants = sizeof(plants) / sizeof(plants[0]);
const uint8_t num_menu_items = 7; // Number of items that fit on the display at once

uint8_t current_plant_index = 0;
uint8_t top_menu_item_index = 0;

unsigned long last_debounce_time = 0;
const unsigned long debounce_delay = 100;

// this is the bitmap for the plant greet screan
//  'selfplanter (1)', 128x64px
const unsigned char epd_bitmap_selfplanter__1_[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x11, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x30, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x23, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x45, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x20, 0xca, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x30, 0x8e, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x48, 0x8c, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x44, 0x84, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xc2, 0xcc, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x82, 0x40, 0x8f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x17, 0x31, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x03, 0x2b, 0x0e, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x39, 0xe8, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x71, 0x98, 0x59, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x51, 0x88, 0xd8, 0x80, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
    0x02, 0x72, 0xcc, 0x94, 0x80, 0x04, 0x11, 0xc2, 0x1c, 0x38, 0xec, 0x38, 0x0f, 0x1c, 0x00, 0x00,
    0x03, 0x02, 0xc4, 0x94, 0x80, 0x04, 0x12, 0x22, 0x22, 0x44, 0x92, 0x44, 0x04, 0x22, 0x00, 0x00,
    0x01, 0x84, 0x40, 0x94, 0xc0, 0x04, 0x92, 0x22, 0x20, 0x44, 0x92, 0x44, 0x04, 0x22, 0x00, 0x00,
    0x00, 0x78, 0xb8, 0x94, 0x80, 0x04, 0x93, 0xe2, 0x20, 0x44, 0x92, 0x7c, 0x04, 0x22, 0x00, 0x00,
    0x00, 0x09, 0xc8, 0x98, 0x80, 0x04, 0x92, 0x02, 0x20, 0x44, 0x92, 0x40, 0x04, 0x22, 0x00, 0x00,
    0x06, 0x0f, 0xc8, 0x81, 0x80, 0x04, 0x92, 0x22, 0x22, 0x44, 0x82, 0x44, 0x04, 0x22, 0x00, 0x00,
    0x08, 0xee, 0x4e, 0x43, 0x00, 0x03, 0x61, 0xc1, 0x1c, 0x38, 0x82, 0x38, 0x03, 0x1c, 0x00, 0x00,
    0x04, 0x1c, 0x4a, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x06, 0x52, 0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x03, 0x73, 0x7d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x01, 0xa3, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x01, 0x23, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x03, 0x01, 0x23, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x80, 0x23, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x81, 0x3c, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x63, 0x08, 0xd8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x1d, 0x08, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x08, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x1f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x10, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x3f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x20, 0x00, 0x04, 0x03, 0xc7, 0xe6, 0x07, 0xe7, 0xc6, 0x03, 0xc6, 0x67, 0xe7, 0xe7, 0xc0,
    0x00, 0x20, 0x00, 0x04, 0x06, 0x66, 0x06, 0x06, 0x06, 0x66, 0x06, 0x66, 0x61, 0x86, 0x06, 0x60,
    0x00, 0x10, 0x00, 0x04, 0x06, 0x06, 0x06, 0x06, 0x06, 0x66, 0x06, 0x66, 0x61, 0x86, 0x06, 0x60,
    0x00, 0x10, 0x00, 0x0c, 0x06, 0x06, 0x06, 0x06, 0x06, 0x66, 0x06, 0x67, 0x61, 0x86, 0x06, 0x60,
    0x00, 0x10, 0x00, 0x0c, 0x03, 0xc7, 0x86, 0x07, 0x87, 0xc6, 0x06, 0x67, 0xe1, 0x87, 0x87, 0xc0,
    0x00, 0x10, 0x00, 0x0c, 0x00, 0x66, 0x06, 0x06, 0x06, 0x06, 0x07, 0xe6, 0xe1, 0x86, 0x07, 0x80,
    0x00, 0x10, 0x00, 0x08, 0x00, 0x66, 0x06, 0x06, 0x06, 0x06, 0x06, 0x66, 0x61, 0x86, 0x06, 0xc0,
    0x00, 0x10, 0x00, 0x08, 0x06, 0x66, 0x06, 0x06, 0x06, 0x06, 0x06, 0x66, 0x61, 0x86, 0x06, 0x60,
    0x00, 0x00, 0x00, 0x08, 0x03, 0xc7, 0xe7, 0xe6, 0x06, 0x07, 0xe6, 0x66, 0x61, 0x87, 0xe6, 0x60,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x09, 0x3f, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x16, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x0f, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

void setup()
{
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  pump.init();
  Serial.begin(9600);
  rtc.begin();
  pinMode(mq135Pin, INPUT);
  pinMode(soilPin, INPUT);
  pinMode(tempRelayPin, OUTPUT);
  pinMode(humRelayPin, OUTPUT);
  pinMode(airQualityRelayPin, OUTPUT);
  pinMode(soilMoistureRelayPin, OUTPUT);
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  light.init();

  // EEPROM.put(0,0);
  EEPROM.get(0, previouslyFertilized);
  EEPROM.get(2, plantSelected);
  EEPROM.get(5, tempThreshold);
  EEPROM.get(10, humThreshold);
  EEPROM.get(15, airQualityThreshold);
  EEPROM.get(20, soilMoistureThreshold);
  EEPROM.get(25, N);
  EEPROM.get(30, P);
  EEPROM.get(35, K);
  plantName = readStringFromEEPROM(40);

  dht.begin(); // initialize the sensor
  display.clearDisplay();
  display.drawBitmap(0, 0, epd_bitmap_selfplanter__1_, 128, 64, WHITE);
  display.display();
  delay(5000);

  display.clearDisplay();
  if (plantSelected == 0)
  {
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("To start ");
    display.println("growing");
    display.println("Select a");
    display.println("Plant:");
    display.display();
  }
  else
  {
    printPlantData();
  }

  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_SELECT, INPUT_PULLUP);
}

void loop()
{
  readThresholdValues();
  light.start(); // begins the lighting process which is fading in and out
  hum = dht.readHumidity();
  temp = dht.readTemperature();

  // read air quality from MQ135 sensor
  ppm = gasSensor.getPPM();

  // callibrate this for soil moisture
  soilMoisture = map(analogRead(soilPin), 520, 210, 0, 100);
  soilMoistureRaw = analogRead(soilPin);

  unsigned long current_time = millis();

  if (current_time - last_debounce_time >= debounce_delay)
  {
    if (digitalRead(BUTTON_UP) == LOW)
    {
      EEPROM.put(2, 0);
      // EEPROM.put(0, 0);
      if (current_plant_index > 0)
      {
        current_plant_index--;
        if (current_plant_index < top_menu_item_index)
        {
          top_menu_item_index--;
        }
        drawMenu();
      }
      last_debounce_time = current_time;
    }
    else if (digitalRead(BUTTON_DOWN) == LOW)
    {
      EEPROM.put(2, 0);
      // EEPROM.put(0, 0);
      if (current_plant_index < num_plants - 1)
      {
        current_plant_index++;
        if (current_plant_index >= top_menu_item_index + num_menu_items)
        {
          top_menu_item_index++;
        }
        drawMenu();
      }
      last_debounce_time = current_time;
    }
    else if (digitalRead(BUTTON_SELECT) == LOW)
    {
      Plant plant = plants[current_plant_index];
      EEPROM.put(2, 1);
      storePlantData(plant);
      printPlantData();
      last_debounce_time = current_time;
    }
  }
  printSensorData();
  relaycontrol();
  delay(10);
}

void drawMenu()
{
  display.setTextSize(1);
  display.stopscroll();
  display.clearDisplay();
  display.setCursor(0, 0);
  for (uint8_t i = 0; i < num_menu_items && top_menu_item_index + i < num_plants; i++)
  {
    uint8_t plant_index = top_menu_item_index + i;
    if (plant_index == current_plant_index)
    {
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    }
    else
    {
      display.setTextColor(SSD1306_WHITE);
    }
    display.println(plants[plant_index].name);
  }
  display.display();
}

void storePlantData(Plant plant)
{
  char *token = strtok(plant.npk_ratio, "-");
  N = atoi(token);
  token = strtok(NULL, "-");
  P = atoi(token);
  token = strtok(NULL, "-");
  K = atoi(token);
  plantName = String(plant.name);
  storeStringToEEPROM(40, plantName);
  EEPROM.put(25, N);
  EEPROM.put(30, P);
  EEPROM.put(35, K);
  tempThreshold = plant.temperature;
  humThreshold = plant.humidity;
  airQualityThreshold = plant.co2_level;
  soilMoistureThreshold = plant.soil_moisture;

  // store all the values in the eeprom
  EEPROM.put(5, tempThreshold);
  EEPROM.put(10, humThreshold);
  EEPROM.put(15, airQualityThreshold);
  EEPROM.put(20, soilMoistureThreshold);
  EEPROM.put(0, 0); // this is to reset the previously fertilized flag
}

void printPlantData()
{
  Serial.begin(9600);
  DateTime now = rtc.now();
  int hour = now.hour();
  int minute = now.minute();
  int seconds = now.second();
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(plantName);
  display.print("   Time: ");
  display.print(hour);
  display.print(":");
  display.println(minute);
  display.print("Humidity = ");
  display.print(humThreshold);
  display.println("%");
  display.print("SoilMoisture = ");
  display.print(soilMoistureThreshold);
  display.println("%");
  display.print("Co2 = ");
  display.print(airQualityThreshold);
  display.println("ppm");
  display.print("Temperature = ");
  display.print(tempThreshold);
  display.println("C");
  display.println("N-P-K Ratio:");
  display.print(N);
  display.print("-");
  display.print(P);
  display.print("-");
  display.println(K);
  display.display();
  // display.startscrollleft(0x00, 0x07);
  if (previouslyFertilized == 0)
  {
    pump.runMotor(N, P, K, pumpRate, totalSolution); // this part fertilizes the plant
    Serial.println("ran the motors");
    EEPROM.put(0, 1);
  }
}

void relaycontrol()
{
  static unsigned long lastMillis = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - lastMillis >= 2000)
  {
    lastMillis = currentMillis;
    // trigger temperature relay if threshold is exceeded
    if (temp > tempThreshold)
    {
      digitalWrite(tempRelayPin, HIGH);
      digitalWrite(airQualityRelayPin, HIGH);
    }
    else
    {
      digitalWrite(tempRelayPin, LOW);
      digitalWrite(airQualityRelayPin, LOW);
    }

    // trigger humidity relay if threshold is exceeded
    if (hum < humThreshold - 5)
    {
      if (hum < humThreshold)
      {
        digitalWrite(humRelayPin, HIGH);
      }
      else
      {
        digitalWrite(humRelayPin, LOW);
      }
    }
    else
    {
      digitalWrite(humRelayPin, LOW);
    }

    // trigger air quality relay if threshold is exceeded
    if (ppm <= airQualityThreshold)
    {
      if (ppm <= airQualityThreshold + 100)
      {
        digitalWrite(airQualityRelayPin, HIGH);
      }
      else
      {
        digitalWrite(airQualityRelayPin, LOW);
      }
    }
    else
    {
      digitalWrite(airQualityRelayPin, LOW);
    }

    // trigger soil moisture relay if threshold is exceeded

    // check if the sensor value is below the threshold
    if (soilMoisture < soilMoistureThreshold - 10)
    {
      if (soilMoisture < soilMoistureThreshold)
      {
        if (!functionCalled)
        {
          // check if this is the first time the sensor value has fallen below the threshold
          if (startTimeforpump == 0)
          {
            startTimeforpump = millis(); // record the start time
          }
          else
          {
            // check if 10 seconds have passed since the start time
            unsigned long currentTimeforpump = millis();
            if (currentTimeforpump - startTimeforpump >= 60000)
            {
              // 10 seconds have passed, call the function
              waterover();
            }
          }
          digitalWrite(soilMoistureRelayPin, HIGH); // turn on the LED
        }
      }
      else
      {
        // reset the start time and functionCalled flag if the sensor value is above the threshold
        startTimeforpump = 0;
        functionCalled = false;
        digitalWrite(soilMoistureRelayPin, LOW); // turn off the LED
      }
    }
    else
    {
      digitalWrite(soilMoistureRelayPin, LOW);
    }
  }
}
void waterover()
{
  functionCalled = true;
  startTimeforpump = 0;
  previouslyFertilized = 0;
  EEPROM.put(0, 0);
  digitalWrite(soilMoistureRelayPin, LOW);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Water level low!!");
  display.println("please refill reset");
  display.println("then reselect plant");
  display.display();
  digitalWrite(soilMoistureRelayPin, LOW);
  while (true)
  {
  }
}

void printSensorData()
{
  static unsigned long lastMillis = 0;
  unsigned long currentMillis = millis();
  // float rzero = gasSensor.getRZero(); // uncomment for ppm callibration

  if (currentMillis - lastMillis >= 2000)
  { // check if 2 seconds have elapsed
    lastMillis = currentMillis;

    // Serial.println(rzero); //uncomment this line if you want to find the new rzero value for callibration purpose
    Serial.print(plantName);
    printf("-%.2f-%.2f-%i-%.2f-%i-%i-%i \n", hum, temp, soilMoisture, ppm, N, P, K);
  }
}
void readThresholdValues()
{
  if (Serial.available() > 0)
  {
    String data = Serial.readStringUntil('\n');
    char *dataCopy = new char[data.length() + 1];
    strcpy(dataCopy, data.c_str());

    char *token = strtok(dataCopy, "-");
    plantName = token;
    token = strtok(NULL, "-");
    tempThreshold = atof(token);
    token = strtok(NULL, "-");
    humThreshold = atof(token);
    token = strtok(NULL, "-");
    airQualityThreshold = atoi(token);
    token = strtok(NULL, "-");
    soilMoistureThreshold = atoi(token);
    token = strtok(NULL, "-");
    N = atoi(token);
    token = strtok(NULL, "-");
    P = atoi(token);
    token = strtok(NULL, "-");
    K = atoi(token);

    delete[] dataCopy;

    EEPROM.put(5, tempThreshold);
    EEPROM.put(10, humThreshold);
    EEPROM.put(15, airQualityThreshold);
    EEPROM.put(20, soilMoistureThreshold);
    EEPROM.put(25, N);
    EEPROM.put(30, P);
    EEPROM.put(35, K);
    EEPROM.put(2, 1);
    storeStringToEEPROM(40, plantName);
    printPlantData();
  }
}

void storeStringToEEPROM(int address, const String &data)
{
  int length = data.length();
  for (int i = 0; i < length; ++i)
  {
    EEPROM.write(address + i, data[i]);
  }
  // Null-terminate the string in EEPROM
  EEPROM.write(address + length, '\0');
}

String readStringFromEEPROM(int address)
{
  String data = "";
  char ch = EEPROM.read(address);
  int i = 0;
  while (ch != '\0' && i < eepromSize)
  {
    data += ch;
    ++i;
    ch = EEPROM.read(address + i);
  }
  return data;
}