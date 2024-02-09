#ifndef LIGHTS_H
#define LIGHTS_H

#include <Arduino.h>
#include <RTClib.h>

class Lights
{
private:
  int pin;
  int fadeDuration; // generally should be 15 mins so that's 15000
  RTC_DS3231 rtc;
  int pwm = 0;

public:
  Lights() {}
  Lights(int pin, int fadeDuration)
  {
    this->pin = pin;
    this->fadeDuration = fadeDuration;
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
    int hour = now.hour();
    int minute = now.minute();
    int seconds = now.second();

    if (hour == 6 && minute >= 0 && minute <= (fadeDuration + 1))
    {
      if (seconds % int(fadeDuration * 60 / 255) == 0)
      {
        analogWrite(pin, pwm);
        if (pwm < 255)
        {
          pwm = pwm + 1;
        }
        else
        {
          pwm = 255;
        }
      }
    }

    if (hour == 18 && minute >= 0 && minute <= (fadeDuration + 1))
    {
      if (seconds % int(fadeDuration * 60 / 255) == 0)
      {
        analogWrite(pin, pwm);
        if (pwm > 0)
        {
          pwm = pwm - 1;
        }
        else
        {
          pwm = 0;
        }
      }
    }
  }
};

#endif
