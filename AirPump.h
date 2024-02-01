#ifndef AIRPUMP_H
#define AIRPUMP_H
#include <Arduino.h>
class AirPump {
private:
  int motorN;
  int motorP;
  int motorK;
  int ratioN;
  int ratioP;
  int ratioK;
  float pumpRate;
  float totalSolution;
  float nitrogenTime;
  float phosphorusTime;
  float potassiumTime;

public:
  AirPump() {}
  AirPump(int motorN, int motorP, int motorK) {
    this->motorN = motorN;
    this->motorP = motorP;
    this->motorK = motorK;
  }

  void init(){
    pinMode(motorN, OUTPUT);
    pinMode(motorP, OUTPUT);
    pinMode(motorK, OUTPUT);
  }

  void runMotor(int ratioN, int ratioP, int ratioK, float pumpRate, float totalSolution){
    float nitrogenSolution = totalSolution * ratioN / 100.0;
    float phosphorusSolution = totalSolution * ratioP / 100.0;
    float potassiumSolution = totalSolution * ratioK / 100.0;

    // Calculate the time to run each pump
    nitrogenTime = 2 + (nitrogenSolution / (pumpRate * 200.0));
    phosphorusTime = 2 + (phosphorusSolution / (pumpRate * 200.0));
    potassiumTime = 2 + (potassiumSolution / (pumpRate * 200.0));
    Serial.println("N P K times");
    Serial.print(nitrogenTime);
    Serial.print(phosphorusTime);
    Serial.print(potassiumTime);


    unsigned long startTime = millis();

    // Run the pumps for the required time to add the desired amount of solution
    while (millis() - startTime < (nitrogenTime + phosphorusTime + potassiumTime) * 1000) {
      if (millis() - startTime < nitrogenTime * 1000) {
        digitalWrite(motorN, HIGH);  // Start the motor 1 pump
      } else {
        digitalWrite(motorN, LOW);  // Stop the motor 1 pump
      }

      if (millis() - startTime < phosphorusTime * 1000) {
        digitalWrite(motorP, HIGH);  // Start the motor 2 pump
      } else {
        digitalWrite(motorP, LOW);  // Stop the motor 2 pump
      }

      if (millis() - startTime < potassiumTime * 1000) {
        digitalWrite(motorK, HIGH);  // Start the motor 3 pump
      } else {
        digitalWrite(motorK, LOW);  // Stop the motor 3 pump
      }
    }
    digitalWrite(motorN, LOW);
    digitalWrite(motorP, LOW);
    digitalWrite(motorK, LOW);
  }

};
#endif