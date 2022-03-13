
#include "trace.h"

#include "encoder.h"
void IRAM_ATTR isrEncoderSwitch();
void IRAM_ATTR isrEncoderTurn();

DRAM_ATTR static volatile bool encoderClick = false;
DRAM_ATTR static volatile bool encoderLongClick = false;
DRAM_ATTR static volatile int16_t encoderRotationCount = 0;

Encoder::Encoder() {};

void Encoder::begin(Display& theScreen)
{
  TRACE();

  screen = theScreen;
  pinMode(encCLK, INPUT_PULLUP);
  pinMode(encDT, INPUT_PULLUP);
  attachInterrupt(encCLK, isrEncoderTurn,   CHANGE);
  attachInterrupt(encDT,  isrEncoderTurn,   CHANGE);
  attachInterrupt(encSW,  isrEncoderSwitch, CHANGE);
}

EncoderEvent Encoder::eventStatus()
{
  if (encoderClick)
  {
    DEB_PL("Encoder click");
    encoderClick = false;
    return EncoderEvent::CLICK;
  }
  if (encoderLongClick)
  {
    DEB_PL("Encoder long click");
    encoderLongClick = false;
    return EncoderEvent::LONGCLICK;
  }
  ticks += encoderRotationCount;
  if (encoderRotationCount != 0)
  {
    encoderRotationCount = 0;
    if (ticks < 0)
    {
      return EncoderEvent::TURN_LEFT;
    }
    else
    {
      return EncoderEvent::TURN_RIGHT;
    }
  }
  return EncoderEvent::NONE;
}

void Encoder::setEncoderEvent(EncoderEvent value)
{
  switch (value)
  {
    case EncoderEvent::NONE:
      encoderClick = false;
      encoderLongClick = false;
      ticks = 0;
      break;
    case EncoderEvent::CLICK:
      encoderClick = true;
      break;
    case EncoderEvent::LONGCLICK:
      encoderLongClick = false;
      break;
    case EncoderEvent::TURN_LEFT:
      ticks = 0;
      encoderRotationCount--;
      break;
    case EncoderEvent::TURN_RIGHT:
      ticks = 0;
      encoderRotationCount++;
      break;
  }
}

int16_t Encoder::getTicks()
{
  int16_t tickValue = ticks;
  ticks = 0;
  return tickValue;
}

//==================================================================================================
// stolen from Edzelf and modified
//==================================================================================================
//**************************************************************************************************
//                                          I S R _ E N C _ S W I T C H                            *
//**************************************************************************************************
// Interrupts received from rotary encoder switch.                                                 *
//**************************************************************************************************
void IRAM_ATTR isrEncoderSwitch()
{
  DRAM_ATTR static volatile uint32_t     oldtime = 0;      // Time in millis previous interrupt
  DRAM_ATTR static volatile bool         sw_state;         // True is pushed (LOW)
  bool            newstate ;                               // Current state of input signal
  uint32_t        newtime ;                                // Current timestamp

  // Read current state of SW pin
  newstate = (digitalRead(encSW) == LOW );
  newtime = millis();
  if ((newtime - oldtime) < encoderSwitchDebounceInterval)   // [x] ms Debounce
  {
    return ;
  }
  if (newstate != sw_state)                                // State changed?
  {
    sw_state = newstate ;                                  // Yes, set current (new) state
    if (!sw_state)                                         // SW released?
    {
      if ((newtime - oldtime) > encoderSwitchLongPressInterval) // More than [x] second?
      {
        encoderLongClick = true ;                          // Yes, register longclick
      }
      else
      {
        encoderClick = true;                               // Yes, click detected
      }
    }
  }
  oldtime = newtime ;                                      // For next compare
}

//**************************************************************************************************
//                                          I S R _ E N C _ T U R N                                *
//**************************************************************************************************
// Interrupts received from rotary encoder (clk signal) knob turn.                                 *
// The encoder is a Manchester coded device, the outcomes (-1,0,1) of all the previous state and   *
// actual state are stored in the enc_states[].                                                    *
// Full_status is a 4 bit variable, the upper 2 bits are the previous encoder values, the lower    *
// ones are the actual ones.                                                                       *
// 4 bits cover all the possible previous and actual states of the 2 PINs, so this variable is     *
// the index enc_states[].                                                                         *
// No debouncing is needed, because only the valid states produce values different from 0.         *
// Rotation is 4 if position is moved from one fixed position to the next, so it is devided by 4.  *
//**************************************************************************************************
void IRAM_ATTR isrEncoderTurn()
{
  DRAM_ATTR static volatile uint32_t     old_state = 0x0001;    // Previous state
  DRAM_ATTR static volatile int16_t      locrotcount = 0;       // Local rotation count
  uint8_t         act_state = 0;                                // The current state of the 2 PINs
  uint8_t         inx;                                          // Index in enc_state
  DRAM_ATTR static volatile const int8_t enc_states [] =                               // Table must be in DRAM (iram safe)
  {
    0,                    // 00 -> 00
    -1,                   // 00 -> 01                           // dt goes HIGH
    1,                    // 00 -> 10
    0,                    // 00 -> 11
    1,                    // 01 -> 00                           // dt goes LOW
    0,                    // 01 -> 01
    0,                    // 01 -> 10
    -1,                   // 01 -> 11                           // clk goes HIGH
    -1,                   // 10 -> 00                           // clk goes LOW
    0,                    // 10 -> 01
    0,                    // 10 -> 10
    1,                    // 10 -> 11                           // dt goes HIGH
    0,                    // 11 -> 00
    1,                    // 11 -> 01                           // clk goes LOW
    -1,                   // 11 -> 10                           // dt goes HIGH
    0                     // 11 -> 11
  } ;
  // Read current state of CLK, DT pin. Result is a 2 bit binary number: 00, 01, 10 or 11.
  act_state  = digitalRead(encCLK) == HIGH ? 0x02 : 0x00;
  act_state |= digitalRead(encDT) == HIGH ? 0x01 : 0x00;
  //act_state = (digitalRead(encCLK) << 1) + digitalRead(encDT);
  inx = (old_state << 2) + act_state;                           // Form index in enc_states
  locrotcount += enc_states[inx];                               // Get delta: 0, +1 or -1
  if (locrotcount == 4)
  {
    encoderRotationCount++;
    locrotcount = 0;
  }
  else if (locrotcount == -4)
  {
    encoderRotationCount--;
    locrotcount = 0;
  }
  old_state = act_state;                                        // Remember current status
}
