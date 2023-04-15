#include "DHT.h"
#define DHTPIN A2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const int mq135Pin = A0;  // Analog input pin connected to the MQ135 gas sensor
float ro = 10.0; // Sensor resistance at 100 ppm of CO2
float curve = 1.7; // CO2 sensitivity of the sensor
float Vout, Rs, ppm;                     // MQ-135 sensor pin
int soilPin = A1;                       // soil moisture sensor pin

unsigned long startTime = 0;
const int ledPin = 13; // assuming the LED is connected to pin 13

int tempThreshold = 25;                 // temperature threshold (in °C)
int humThreshold = 40;                  // humidity threshold (in %)
int airQualityThreshold = 1800;          // air quality threshold
int soilMoistureThreshold = 70;        // soil moisture threshold

int tempRelayPin = 11;                   // temperature relay pin
int humRelayPin = 6;                    // humidity relay pin
int airQualityRelayPin = 12;             // air quality relay pin
int soilMoistureRelayPin = 10;           // soil moisture relay pin

const int motor1Pin = 9;  // Pin connected to the motor 1 pump
const int motor2Pin = 8;  // Pin connected to the motor 2 pump
const int motor3Pin = 7;  // Pin connected to the motor 3 pump
const float pumpRate = 5.0 / 1000.0;  // Pump rate in liters per second

void setup() {
  Serial.begin(9600);                   // initialize serial communication

  pinMode(tempRelayPin, OUTPUT);        // set temperature relay pin as output
  pinMode(humRelayPin, OUTPUT);         // set humidity relay pin as output
  pinMode(airQualityRelayPin, OUTPUT);  // set air quality relay pin as output
  pinMode(soilMoistureRelayPin, OUTPUT);// set soil moisture relay pin as output

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH); // start with LED off

  dht.begin(); // initialize the sensor
  runWaterPumps();
}



void loop() {
  turnOnLedFor12Hours();
  // read temperature and humidity from DHT11 sensor
  // put your main code here, to run repeatedly:
  float hum  = dht.readHumidity();
  // read temperature as Celsius
  float temp = dht.readTemperature();

  int sensorValue = analogRead(mq135Pin);  // Read the MQ135 sensor value
  Vout = sensorValue * (5.0 / 1023.0); // Convert sensor value to voltage
  Rs = ((5.0 - Vout) / Vout) * ro; // Calculate sensor resistance

  ppm = pow(10, (log10(Rs / ro) - curve) / 0.45 + 1.67); // Convert sensor resistance to ppm of CO2


  // read soil moisture from capacitive sensor
  int soilMoisture = map(analogRead(soilPin), 1023, 300, 0, 100);

  // print the readings on serial monitor
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print(" °C, Humidity: ");
  Serial.print(hum);
  Serial.print("CO2 concentration: ");
  Serial.print(ppm);
  Serial.println(" ppm");
  Serial.print(", Soil Moisture: ");
  Serial.println(soilMoisture);

  // trigger temperature relay if threshold is exceeded
  if(temp < tempThreshold){
    digitalWrite(tempRelayPin, HIGH);
    digitalWrite(airQualityRelayPin, HIGH);
  }else{
    digitalWrite(tempRelayPin, LOW);
    digitalWrite(airQualityRelayPin, LOW);
  }

  // trigger humidity relay if threshold is exceeded
  if(hum < humThreshold){
  
    digitalWrite(humRelayPin, HIGH);
  }else{
    digitalWrite(humRelayPin, LOW);
  }

  // trigger air quality relay if threshold is exceeded
  if(ppm <= airQualityThreshold){
    digitalWrite(airQualityRelayPin, HIGH);
  }else{
    digitalWrite(airQualityRelayPin, LOW);
  }

  // trigger soil moisture relay if threshold is exceeded
  if(soilMoisture > soilMoistureThreshold){
  unsigned long timeoutmotor = millis() + 30000;  // Timeout of 30 seconds
  while (timeoutmotor > millis()){
    digitalWrite(soilMoistureRelayPin, HIGH);}
  }else{
    digitalWrite(soilMoistureRelayPin, LOW);
  }

  delay(1000);                          // wait for 1 second before taking next readings
}

void turnOnLedFor12Hours() {
  if (millis() - startTime < 12L * 3600L * 1000L) {
    digitalWrite(ledPin, LOW);
  } else {
    digitalWrite(ledPin, HIGH);
  }
}

void runWaterPumps() {
   // Prompt the user to enter the desired ratio of each element
  Serial.println("Enter the ratio of nitrogen, phosphorus, and potassium (in %), separated by commas:");

  // Wait for input from the user with a timeout of 5 seconds
  String inputString = "";
  unsigned long timeout = millis() + 5000;  // Timeout of 5 seconds
  while (timeout > millis() && inputString.indexOf('\n') == -1) {
    if (Serial.available()) {
      inputString += Serial.readString();
    }
  }

  // If no input was received, exit the function
  if (inputString.length() == 0) {
    return;
  }

  // Parse the input string to get the desired ratios
  float nitrogenRatio = inputString.substring(0, inputString.indexOf(',')).toFloat();
  inputString.remove(0, inputString.indexOf(',') + 1);
  float phosphorusRatio = inputString.substring(0, inputString.indexOf(',')).toFloat();
  inputString.remove(0, inputString.indexOf(',') + 1);
  float potassiumRatio = inputString.toFloat();

  // Calculate the amount of solution to add for each element (in ml)
  float totalSolution = 5000.0;  // Total volume of the main tank in ml
  float nitrogenSolution = totalSolution * nitrogenRatio / 100.0;
  float phosphorusSolution = totalSolution * phosphorusRatio / 100.0;
  float potassiumSolution = totalSolution * potassiumRatio / 100.0;

  // Run the pumps for the required time to add the desired amount of solution
  float nitrogenTime = nitrogenSolution / (pumpRate * 2.0);
  float phosphorusTime = phosphorusSolution / (pumpRate * 2.0);
  float potassiumTime = potassiumSolution / (pumpRate * 2.0);

  digitalWrite(motor1Pin, HIGH);  // Start the motor 1 pump
  delay(nitrogenTime * 1000.0);  // Wait for the required time
  digitalWrite(motor1Pin, LOW);   // Stop the motor 1 pump

  digitalWrite(motor2Pin, HIGH);  // Start the motor 2 pump
  delay(phosphorusTime * 1000.0);  // Wait for the required time
  digitalWrite(motor2Pin, LOW);   // Stop the motor 2 pump

  digitalWrite(motor3Pin, HIGH);  // Start the motor 3 pump
  delay(potassiumTime * 1000.0);  // Wait for the required time
  digitalWrite(motor3Pin, LOW);   // Stop the motor 3 pump
}
