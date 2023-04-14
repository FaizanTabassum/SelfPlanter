#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define BUTTON_UP 2
#define BUTTON_DOWN 3
#define BUTTON_SELECT 5

struct Plant {
  const char* name;
  uint8_t humidity;
  uint8_t soil_moisture;
  float temperature;
  uint16_t co2_level;
  const char* npk_ratio;
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
  {"Carrots", 50, 60, 18.0, 500, "1-2-2"}
};

const uint8_t num_plants = sizeof(plants) / sizeof(plants[0]);
const uint8_t num_menu_items = 3; // Number of items that fit on the display at once

uint8_t current_plant_index = 0;
uint8_t top_menu_item_index = 0;

unsigned long last_debounce_time = 0;
const unsigned long debounce_delay = 50;

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Select a plant:");
  display.display();

  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_SELECT, INPUT_PULLUP);
}

void drawMenu() {
  display.stopscroll();
  display.clearDisplay();
  display.setCursor(0, 0);
  for (uint8_t i = 0; i < num_menu_items && top_menu_item_index + i < num_plants; i++) {
    uint8_t plant_index = top_menu_item_index + i;
    if (plant_index == current_plant_index) {
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }
    display.println(plants[plant_index].name);
  }
  display.display();
}

void printPlantData(Plant plant) {
  Serial.begin(9600);
  Serial.print("Name: ");
  Serial.println(plant.name);
  Serial.print("Humidity: ");
  Serial.print(plant.humidity);
  Serial.println("%");
  Serial.print("Soil Moisture: ");
  Serial.print(plant.soil_moisture);
  Serial.println("%");
  Serial.print("Temperature: ");
  Serial.print(plant.temperature);
  Serial.println("°C");
  Serial.print("CO2 Level: ");
  Serial.print(plant.co2_level);
  Serial.println("ppm");
  Serial.print("NPK Ratio: ");
  Serial.println(plant.npk_ratio);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(plant.name);
  display.print(" hum = ");display.print(plant.humidity);display.println("%");
  display.print(" sm = ");display.print(plant.soil_moisture);display.println("%");
  display.print(" temp = ");display.print(plant.temperature);display.println("C");
  display.display();
  display.startscrollleft(0x00, 0x07);

}

void loop() {
  unsigned long current_time = millis();
  
  // Check if enough time has elapsed for button debounce
  if (current_time - last_debounce_time >= debounce_delay) {
    // Button debounce time has elapsed, so check button presses
    if (digitalRead(BUTTON_UP) == LOW) {
      if (current_plant_index > 0) {
        current_plant_index--;
        if (current_plant_index < top_menu_item_index) {
          top_menu_item_index--;
        }
        drawMenu();
      }
      last_debounce_time = current_time;
    } else if (digitalRead(BUTTON_DOWN) == LOW) {
      if (current_plant_index < num_plants - 1) {
        current_plant_index++;
        if (current_plant_index >= top_menu_item_index + num_menu_items) {
          top_menu_item_index++;
        }
        drawMenu();
      }
      last_debounce_time = current_time;
    } else if (digitalRead(BUTTON_SELECT) == LOW) {
      Plant plant = plants[current_plant_index];
      printPlantData(plant);
      last_debounce_time = current_time;
    }
  }
}
