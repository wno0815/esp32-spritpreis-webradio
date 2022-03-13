#pragma once
#include <Arduino.h>

#include "trace.h"
#include "config.h"

#include <time.h>


class Clock
{
  public:
    Clock();

    void begin();
    uint8_t getHour();
    uint8_t getMinute();
    uint8_t getSecond();
    char* getDate();
    const char* getWeekday();
    bool secondEventStatus();
    bool ntpEventStatus();
    bool fuelEventStatus();
    void setSecondEvent();
    void setNtpEvent();
    void setFuelEvent();
    void off();
    bool isOn();
    void forceUpdate();

    bool resetFuelEventInterval = false;

  private:
    TaskHandle_t clockTaskHandle = NULL;
    bool secondEvent = false;
    bool ntpEvent = false;
    bool fuelEvent = false;
    char dateText[11];    // dd.mm.yyyy
    bool statusOn = true;
};
