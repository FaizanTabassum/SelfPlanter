#include "LibPrintf.h"
#include "MQ135.h"
#include "DHT.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include "RTClib.h"
#include "lights.h"
#include "AirPump.h"

#define DHTPIN 3
#define RLOAD 22.0
#define DHTTYPE DHT11
#define LIGHTPIN 13
const int LED_PIN = 13;
const int mq135Pin = A0;       // Analog input pin connected to the MQ135 gas sensor
int soilPin = A1;              // soil moisture sensor pin
int tempRelayPin = 11;         // temperature relay pin
int humRelayPin = 6;           // humidity relay pin
int airQualityRelayPin = 12;   // air quality relay pin
int soilMoistureRelayPin = 10; // soil moisture relay pin

DHT dht(DHTPIN, DHTTYPE);

Lights light(LIGHTPIN, 1500);

MQ135 gasSensor = MQ135(mq135Pin);

float hum = 0;
int soilMoisture = 0;
float temp = 0;

// co2 sensor
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
char *plantName;           // store in address 40 onwards

// npk values for the airpumps
int N; // store in EEPROM address 25
int P; // store in EEPROM address 30
int K; // store in EEPROM address 35
int previouslyFertilized;
const int motorN = 7;                // Pin connected to the motor 1 pump
const int motorP = 8;                // Pin connected to the motor 2 pump
const int motorK = 9;                // Pin connected to the motor 3 pump
const float pumpRate = 5.0 / 1000.0; // Pump rate in liters per second
const float totalSolution = 100.0;   // Total solution in liters

int plantSelected;

AirPump pump(motorN, motorP, motorK);
RTC_DS3231 rtc;

// Oled display parameters
#define OLED_RESET A3
Adafruit_SSD1306 display(OLED_RESET);
#define BUTTON_UP 0
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
const uint8_t num_menu_items = 4; // Number of items that fit on the display at once

uint8_t current_plant_index = 0;
uint8_t top_menu_item_index = 0;

unsigned long last_debounce_time = 0;
const unsigned long debounce_delay = 50;

void setup()
{
  light.init();
  pump.init();
  Serial.begin(9600);
  Wire.begin();
  rtc.begin();                           // initialize serial communication
  pinMode(mq135Pin, INPUT);              // input for the mq135 pin
  pinMode(soilPin, INPUT);               // input for the soil pin
  pinMode(tempRelayPin, OUTPUT);         // set temperature relay pin as output
  pinMode(humRelayPin, OUTPUT);          // set humidity relay pin as output
  pinMode(airQualityRelayPin, OUTPUT);   // set air quality relay pin as output
  pinMode(soilMoistureRelayPin, OUTPUT); // set soil moisture relay pin as output
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
  Serial.println("printing all the stored eeprom values");
  Serial.println(previouslyFertilized);
  Serial.println(plantSelected);
  Serial.println(tempThreshold);
  Serial.println(humThreshold);
  Serial.println(airQualityThreshold);
  Serial.println(soilMoistureThreshold);
  Serial.println(N);
  Serial.println(P);
  Serial.println(K);

  dht.begin(); // initialize the sensor
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  if (plantSelected == 0)
  {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("To start growing");
    display.println("Select a plant:");
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

  light.start(); // begins the lighting process which is fading in and out
  hum = dht.readHumidity();
  temp = dht.readTemperature();

  // read air quality from MQ135 sensor
  ppm = gasSensor.getPPM();

  // callibrate this for soil moisture
  soilMoisture = map(analogRead(soilPin), 750, 350, 0, 100);

  unsigned long current_time = millis();

  if (current_time - last_debounce_time >= debounce_delay)
  {
    if (digitalRead(BUTTON_UP) == LOW)
    {
      EEPROM.put(2, 0);
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
  plantName = plant.name;
  EEPROM.put(25, N);
  EEPROM.put(30, P);
  EEPROM.put(35, K);
  tempThreshold = plant.temperature;     // temperature threshold (in °C)
  humThreshold = plant.humidity;         // humidity threshold (in %)
  airQualityThreshold = plant.co2_level; // air quality threshold
  soilMoistureThreshold = plant.soil_moisture;
  EEPROM.put(5, tempThreshold); // store all the values in the eeprom
  EEPROM.put(10, humThreshold);
  EEPROM.put(15, airQualityThreshold);
  EEPROM.put(20, soilMoistureThreshold);
  EEPROM.put(0, 0); // this is to reset the previously fertilized flag
}

void printPlantData()
{

  if (previouslyFertilized == 0)
  {
    pump.runMotor(N, P, K, pumpRate, totalSolution); // this part fertilizes the plant
    Serial.println("ran the motors");
    EEPROM.put(0, 1);
  }
  Serial.begin(9600);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(plantName);
  display.print("hum = ");
  display.print(humThreshold);
  display.print("%");
  display.print("    sm = ");
  display.print(soilMoistureThreshold);
  display.println("%");
  display.print("Co2 = ");
  display.print(airQualityThreshold);
  display.println("ppm");
  display.print("temp = ");
  display.print(tempThreshold);
  display.println("C");
  display.display();
  // display.startscrollleft(0x00, 0x07);
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
    printf("%.2f-%.2f-%i-%.2f \n", hum, temp, soilMoisture, ppm);
  }
}
