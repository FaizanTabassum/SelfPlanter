#ifndef LIGHTS_H
#define LIGHTS_H

#include <Arduino.h>
#include <RTClib.h>
#define FADE_STEPS 100
#define BRIGHTNESS_MAX 255

class Lights
{
private:
  int pin;
  int fadeDuration; // generally should be 15 mins so that's 15000
  RTC_DS3231 rtc;
  int pwm = 0;
  bool fadeInProgress = false;
  bool fadeOutProgress = false;
  unsigned long fadeStartTime;
  int fadeDirection;
  int currentBrightness = 0;

public:
  Lights() {}
  Lights(int pin, int fadeDuration)
  {
    this->pin = pin;
    this->fadeDuration = fadeDuration;
    this->currentBrightness = currentBrightness;
    this->fadeInProgress = fadeInProgress;
    this->fadeOutProgress = fadeOutProgress;
    this->fadeStartTime = fadeStartTime;
    this->fadeDirection = fadeDirection;
  }
  void init()
  {
    pinMode(pin, OUTPUT);
    rtc.begin();
    DateTime now = rtc.now();
    int hour = now.hour();
    if (hour > 6 && hour < 18)
    {
      pwm = 255;
      analogWrite(pin, 255);
    }

    if (hour >= 18 || hour < 6)
    {
      analogWrite(pin, 0);
      pwm = 0;
    }
  }
  void start()
  {
    DateTime now = rtc.now();

    if (now.hour() == 6 && now.minute() == 0 && !fadeInProgress)
    {
      fadeInProgress = true;
      fadeStartTime = millis();
      fadeDirection = 1;
    }
    else if (now.hour() == 18 && now.minute() == 0 && !fadeOutProgress)
    {
      fadeOutProgress = true;
      fadeStartTime = millis();
      fadeDirection = -1;
    }

    if (fadeInProgress || fadeOutProgress)
    {
      unsigned long elapsedTime = millis() - fadeStartTime;
      int brightnessChange = BRIGHTNESS_MAX / FADE_STEPS;
      int stepsCompleted = min(FADE_STEPS, elapsedTime / ((fadeDuration * 60 * 1000) / FADE_STEPS));

      if (fadeDirection == 1)
      {
        currentBrightness = min(BRIGHTNESS_MAX, brightnessChange * stepsCompleted);
      }
      else if (fadeDirection == -1)
      {
        currentBrightness = max(0, BRIGHTNESS_MAX - (brightnessChange * stepsCompleted));
      }

      analogWrite(pin, currentBrightness);

      if (stepsCompleted == FADE_STEPS)
      {
        fadeInProgress = false;
        fadeOutProgress = false;
      }
    }
  }
};

#endif
