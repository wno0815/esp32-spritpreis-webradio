#include <Arduino.h>

#include "trace.h"
#include "config.h"

#include "display.h"

enum class EncoderEvent { NONE, CLICK, LONGCLICK, TURN_LEFT, TURN_RIGHT };

class Encoder
{
  public:
    Encoder();

    void begin(Display& theScreen);
    EncoderEvent eventStatus();
    int16_t getTicks();
    void setEncoderEvent(EncoderEvent value);   // needed only for test using key in PuTTY

  private:
    Display screen;
    int16_t ticks;
};
