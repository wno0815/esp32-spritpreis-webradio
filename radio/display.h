#pragma once

#include <Arduino.h>

#include "trace.h"
#include "config.h"

enum class ButtonEvent { NONE, KEY, PREVIOUS, NEXT, LEFT, RIGHT, DARK, BRIGHT, MIDDLE, LIMITS };
enum class ValueType { LIMIT_DIESEL, LIMIT_SUPER };


class Display
{
  public:
    Display();

    void begin(uint8_t rx = nextionRXD, uint8_t tx = nextionTXD);
    // Nextion page 0: Debug
    void selectPage(Pages page);
    Pages getCurrentPage();
    void debug(const char* text, bool appendText = false);
    void debug(const int32_t value, bool appendValue = false);
    void debug(const size_t value, bool appendValue = false);

    // Nextion page 1: Radio station display
    void setStation(const char* stationName);
    void setTitle(const char* titleText);
    void setKeyText(const uint8_t key, const char* text);
    void deactivateKeys(const uint8_t number);
    void activateKey(const uint8_t key, const bool activate);

    // Nextion page 2: Clock
    void setClockTime(const uint8_t hh, const uint8_t mm, const uint8_t ss, const char* dateText, const char* weekdayText);
    void incrementClockSecond();

    // Nextion page 3 and 4: Fuel prices and limits
    void setFuelData(const char* stationName, const float priceDiesel, const float priceSuper, const bool isOpen);
    void setFuelPrices(const float priceDiesel, const float priceSuper, const bool isOpen);
    void setFuelLimits(const int32_t limitDiesel, const int32_t limitSuper);
    void requestValue(ValueType value);
    int32_t getReceivedValue();
    void setFuelAlarmDiesel(const bool activate);
    void setFuelAlarmSuper(const bool activate);

    // Nextion page 5: Download
    void setType(const char* typeText);
    void setProgress(const byte value);

    // Nextion receive
    void setButtonEvent(ButtonEvent value);
    void setButtonKey(uint8_t key);
    char* getReceiveBuffer();
    ButtonEvent buttonEventStatus();
    uint8_t getButtonKey();

    // settings and others
    void endCommand();
    void off();
    int32_t setBrightness(int32_t value);
    int32_t adjustBrightness(bool brighter);

    // TODO: Clean up access for task - this should be private
    int32_t lastReceivedValue = -1;
    // TODO: use event here
    bool valueReceived = false;

  private:
    void adjustDebugLine(bool append);

    uint8_t rxPin = nextionRXD;
    uint8_t txPin = nextionTXD;

    char receiveBuffer[::nextionBufferSize];
    TaskHandle_t receiveTaskHandle = NULL;
    ButtonEvent buttonEvent = ButtonEvent::NONE;
    uint8_t buttonKey;

    Pages currentPage = Pages::DEBUG;
    int8_t currentDebugLine = 0;

    int brightness;
    void convertUtf8toIso(const char* text, uint8_t* textBuffer);

};
