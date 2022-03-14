
#include "trace.h"

// needed this for checking the conversion code
//#define VERBOSE_UTF_ISO_CONVERSION
#include "display.h"

SemaphoreHandle_t displayMutex;
void displayReceiveTask(void* parameters);

Display::Display() {};

void Display::begin(uint8_t rx, uint8_t tx)
{
  TRACE();

  rxPin = rx;
  txPin = tx;

  Serial2.begin(9600, SERIAL_8N1, rxPin, txPin);
  delay(100);

  displayMutex = xSemaphoreCreateMutex();
  xTaskCreate(    displayReceiveTask,   /* Task function. */
                  "NextRecv",           /* String with name of task. */
                  4096,                 /* Stack size in bytes. */
                  (void*)this,          /* Parameter passed as input of the task: This Display object*/
                  1,                    /* Priority of the task. */
                  &receiveTaskHandle);  /* Task handle. */
  delay(100);
  DEB_PF("receive task created; buffer size is %d\n", ::nextionBufferSize);

  // clear debug screen
  selectPage(Pages::DEBUG);
  delay(100);
}


void Display::selectPage(Pages page)
{
  TRACE();
  Serial2.print("page ");
  switch (page)
  {
    case Pages::DEBUG:
      DEB_PL("page DEBUG");
      Serial2.print(0);
      break;
    case Pages::PLAYER:
      Serial2.print(1);
      DEB_PL("page PLAYER");
      break;
    case Pages::CLOCK:
      Serial2.print(2);
      DEB_PL("page CLOCK");
      break;
    case Pages::FUEL:
      Serial2.print(3);
      DEB_PL("page FUEL");
      break;
    // fuel limits is page #4
    case Pages::DOWNLOAD:
      DEB_PL("page DOWNLOAD");
      Serial2.print(5);
      break;
  }
  endCommand();
  currentPage = page;
}

void Display::off()
{
  if (receiveTaskHandle != NULL)
  {
    // responses from display are handled in update
    xSemaphoreTake(displayMutex, portMAX_DELAY);
    vTaskDelete(receiveTaskHandle);
    receiveTaskHandle = NULL;
    xSemaphoreGive(displayMutex);
  }
  DEB_PL("display is off");
}

Pages Display::getCurrentPage()
{
  return currentPage;
}


void Display::debug(const char* text, bool appendText)
{
  if (currentPage == Pages::DEBUG)
  {
    adjustDebugLine(appendText);
    Serial2.printf("t%d.txt%s=\"%s\"", currentDebugLine, (appendText ? "+" : ""), text);
    endCommand();
    currentDebugLine++;
  }
  DEB_PF("DISPLAY%c debug '%s'\n", (appendText ? '+' : ':'), text);
}

void Display::debug(const int32_t value, bool appendValue)
{
  if (currentPage == Pages::DEBUG)
  {
    adjustDebugLine(appendValue);
    Serial2.printf("t%d.txt%s=\"%d\"", currentDebugLine, (appendValue ? "+" : ""), value);
    endCommand();
    currentDebugLine++;
  }
  DEB_PF("DISPLAY%c debug '%d'\n", (appendValue ? '+' : ':'), value);
}

void Display::debug(const size_t value, bool appendValue)
{
  if (currentPage == Pages::DEBUG)
  {
    adjustDebugLine(appendValue);
    Serial2.printf("t%d.txt%s=\"%zu\"", currentDebugLine, (appendValue ? "+" : ""), value);
    endCommand();
    currentDebugLine++;
  }
  DEB_PF("DISPLAY%c debug '%zu'\n", (appendValue ? '+' : ':'), value);
}

void Display::adjustDebugLine(bool append)
{
  if (!append)
  {
    // need to check if we are already in last line
    if (currentDebugLine == numberOfDebugLines)
    {
      // move all texts one line up
      Serial2.printf("click scroll,1");
      endCommand();
      currentDebugLine = numberOfDebugLines - 1;
    }
  }
  else
  {
    // go back to previous line
    if (currentDebugLine)
      currentDebugLine--;
  }
}

void Display::endCommand()
{
  Serial2.write(0xFF);
  Serial2.write(0xFF);
  Serial2.write(0xFF);
}

void Display::setStation(const char* stationName)
{
  TRACE();
  static uint8_t textBuffer[256];
  convertUtf8toIso(stationName, textBuffer);

  switch (currentPage)
  {
    case Pages::PLAYER:
    case Pages::FUEL:
      Serial2.printf("station.txt=\"%s\"", textBuffer);
      endCommand();
      DEB_PF("DISPLAY: set stationName '%s'\n", textBuffer);
      break;
    case Pages::DEBUG:
    case Pages::CLOCK:
    case Pages::DOWNLOAD:
      break;
  }
}

void Display::convertUtf8toIso(const char* text, uint8_t* textBuffer)
{
  // Due to smaller size the fonts in the Nextion UI are using ISO8859-15 encoding.
  // It seems that titleText is delivered in UTF-8 bei VS1053 library.
  // Whole file 'tanken.json' is encoded UTF-8 (fuel station names).
  // Therefore conversion needed.
  // see also https://stackoverflow.com/questions/53269432/convert-from-utf-8-to-iso8859-15-in-c
  int32_t src = 0;
  int32_t dst = 0;

  uint8_t ch = (uint8_t)(text[src]);
  uint32_t codepoint = 0;
  while (ch != 0)
  {
    if (ch <= 0x7F)
      codepoint = ch;
    else if (ch <= 0xBF)
      codepoint = (codepoint << 6) | (ch & 0x3f);
    else if (ch <= 0xDF)
      codepoint = ch & 0x1F;
    else if (ch <= 0xef)
      codepoint = ch & 0x0F;
    else
      codepoint = ch & 0x07;

#if defined (VERBOSE_UTF_ISO_CONVERSION)
    DEB_PF("\nsrc=%3d dst=%3d ch=0x%2.2X [%c]  cp=0x%8.8X   ", src, dst, ch, (ch < 0x80 ? ch : '.'), codepoint);
#endif

    src++;
    ch = (uint8_t)(text[src]);

    if (((ch & 0xC0) != 0x80) && (codepoint <= 0x10FFFF))
    {
      // a valid codepoint has been decoded; convert it to ISO-8859-15

      uint8_t outc;
      if (codepoint <= 255)
      {
        switch (codepoint)
        {
          case 0xA4:
          case 0xA6:
          case 0xA8:
          case 0xB4:
          case 0xB8:
          case 0xBC:
          case 0xBD:
          case 0xBE:
            outc = '?';
            break;
          default:
            outc = (uint8_t)codepoint;
            break;
        }
#if defined (VERBOSE_UTF_ISO_CONVERSION)
        DEB_PF("  cp=0x%4.4X  outc=0x%2.2X [%c]", codepoint, outc, (outc < 0x80 ? outc : '.'));
#endif
      }
      else
      {
        // With a few exceptions, codepoints above 255 cannot be converted
        switch (codepoint)
        {
          case 0x20AC:
            outc = 0xA4;
            break;
          case 0x0160:
            outc = 0xA6;
            break;
          case 0x0161:
            outc = 0xA8;
            break;
          case 0x017d:
            outc = 0xB4;
            break;
          case 0x017e:
            outc = 0xB8;
            break;
          case 0x0152:
            outc = 0xBC;
            break;
          case 0x0153:
            outc = 0xBD;
            break;
          case 0x0178:
            outc = 0xBE;
            break;
          default:
            outc = (uint8_t)'?';
            break;
        }
#if defined (VERBOSE_UTF_ISO_CONVERSION)
        DEB_PF("R cp=0x%4.4X  outc=0x%2.2X [%c]", codepoint, outc, (outc < 0x80 ? outc : '.'));
#endif
      }
      switch (outc)
      {
        // need to escape two characters
        case 0x22: //
          textBuffer[dst] = 0x5C;
          dst++;
          break;
        case 0x5C:
          textBuffer[dst] = 0x5C;
          dst++;
          break;
      }
      textBuffer[dst] = outc;
      dst++;
    }
  }
  textBuffer[dst] = 0;
}


void Display::setTitle(const char* titleText)
{
  TRACE();

  static uint8_t textBuffer[256];
  convertUtf8toIso(titleText, textBuffer);

  if (currentPage == Pages::PLAYER)
  {
    Serial2.printf("title.txt=\"%s\"", (char*)textBuffer);
    endCommand();
  }
#if defined (VERBOSE_UTF_ISO_CONVERSION)
  DEB_PL();
#endif
  DEB_PF("DISPLAY: player.title '%s'\n", textBuffer);
}

void Display::setKeyText(const uint8_t key, const char* text)
{
  if (currentPage == Pages::PLAYER)
  {
    Serial2.printf("key%d.txt=\"%s\"", key, text);
    endCommand();
  }
  DEB_PF("DISPLAY: player.key key=%d '%s'\n", key, text);
}

void Display::deactivateKeys(const uint8_t number)
{
  DEB_PL("DISPLAY: deactivate all keys");
  if (currentPage == Pages::PLAYER)
  {
    for (uint8_t count = 1; count < number; count++)
    {
      Serial2.printf("key%d.bco=%d", count, 19049);
      endCommand();
    }
  }
}

void Display::activateKey(const uint8_t key, const bool activate)
{
  if (currentPage == Pages::PLAYER)
  {
    // TODO: get colors from display
    Serial2.printf("key%d.bco=%d", key, activate ? 396 : 19049);
    endCommand();
  }
  DEB_PF("DISPLAY: player.activateKey index=%d '%s'\n", key, activate ? "ACTIVE" : "inactive");
}

int32_t Display::adjustBrightness(bool brighter)
{
  if (brighter)
  {
    brightness += brightnessInterval;
  }
  else
  {
    brightness -= brightnessInterval;
  }
  // send to display
  DEB_PF("brightness %screased to %d\n", brighter ? "in" : "de", brightness);
  setBrightness(brightness);

  return brightness;
}

int32_t Display::setBrightness(int32_t value)
{
  int32_t previousValue = brightness;

  // limit value to 0...100
  brightness = (value < 0 ? 0 : (value > 100 ? 100 : value));

  // send to display
  DEB_PF("brightness set to %d\n", brightness);
  Serial2.printf("dim=%d", brightness);
  endCommand();

  return previousValue;
}


void Display::setButtonEvent(ButtonEvent value)
{
  buttonEvent = value;
}

void Display::setButtonKey(uint8_t key)
{
  buttonKey = key;
}

char* Display::getReceiveBuffer()
{
  return receiveBuffer;
}

ButtonEvent Display::buttonEventStatus()
{
  if (buttonEvent != ButtonEvent::NONE)
  {
    TRACE();
    ButtonEvent lastEvent = buttonEvent;
    buttonEvent = ButtonEvent::NONE;
    xSemaphoreGive(displayMutex);
    return lastEvent;
  }
  return ButtonEvent::NONE;
}

uint8_t Display::getButtonKey()
{
  return buttonKey;
}


void Display::setClockTime(const uint8_t hh, const uint8_t mm, const uint8_t ss, const char* dateText, const char* weekdayText)
{
  TRACE();
  if (currentPage == Pages::CLOCK)
  {
    Serial2.printf("timeHour.val=%d", hh);
    endCommand();
    Serial2.printf("timeMinute.val=%d", mm);
    endCommand();
    Serial2.printf("timeSecond.val=%d", ss);
    endCommand();
    Serial2.printf("date.txt=\"%s\"", dateText);
    endCommand();
    Serial2.printf("weekday.txt=\"%s\"", weekdayText);
    endCommand();
    DEB_PF("time set to %2.2d:%2.2d:%2.2d  %s  %s\n", hh, mm, ss, dateText, weekdayText);
  }
}

void Display::incrementClockSecond()
{
  if (currentPage == Pages::CLOCK)
  {
    Serial2.printf("click timeSecond,1");
    endCommand();
  }
}


void Display::setFuelData(const char* stationName, const float priceDiesel, const float priceSuper, const bool isOpen)
{
  TRACE();
  if (currentPage == Pages::FUEL)
  {
    setStation(stationName);
    setFuelPrices(priceDiesel, priceSuper, isOpen);
    DEB_PF("fuel data set to station='%s' priceDiesel=%5.3f priceSuper=%5.3f\n", stationName, priceDiesel, priceSuper);
  }
}

void Display::setFuelPrices(const float priceDiesel, const float priceSuper, const bool isOpen)
{
  char outbuf[6];
  if (currentPage == Pages::FUEL)
  {
    if (isOpen)
    {
      sprintf(outbuf, "%5.3f", priceDiesel);
      Serial2.printf("pDieselLast.txt=\"%c\"", outbuf[4]);
      endCommand();
      outbuf[4] = 0;
      Serial2.printf("priceDiesel.txt=\"%s\"", outbuf);
      endCommand();
      
      sprintf(outbuf, "%5.3f", priceSuper);
      Serial2.printf("pSuperLast.txt=\"%c\"", outbuf[4]);
      endCommand();
      outbuf[4] = 0;
      Serial2.printf("priceSuper.txt=\"%s\"", outbuf);
      endCommand();

      DEB_PF("update fuel prices priceDiesel=%5.3f priceSuper=%5.3f\n", priceDiesel, priceSuper);
    }
    else
    {
      Serial2.printf("pDieselLast.txt=\" \"");
      endCommand();
      Serial2.printf("priceDiesel.txt=\"---\"");
      endCommand();
      Serial2.printf("pSuperLast.txt=\" \"");
      endCommand();
      Serial2.printf("priceSuper.txt=\"---\"");
      endCommand();
      
      DEB_PF("station closed; remove fuel prices\n");
    }
  }
}

void Display::setFuelLimits(const int32_t limitDiesel, const int32_t limitSuper)
{
  // global variables - independent from current page
  Serial2.printf("currentLimitDiesel=%d", limitDiesel);
  endCommand();
  Serial2.printf("currentLimitSuper=%d", limitSuper);
  endCommand();
}

void Display::setFuelAlarmDiesel(const bool activate)
{
  Serial2.printf("click priceDiesel,%c", activate ? '1' : '0');
  endCommand();
}

void Display::setFuelAlarmSuper(const bool activate)
{
  Serial2.printf("click priceSuper,%c", activate ? '1' : '0');
  endCommand();
}

void Display::setType(const char* typeText)
{
  Serial2.printf("type.txt=\"%s\"", typeText);
  endCommand();
}

void Display::setProgress(const byte value)
{
  Serial2.printf("progress.val=%d", value > 100 ? 100 : value);
  endCommand();
}



void Display::requestValue(ValueType value)
{
  switch (value)
  {
    case ValueType::LIMIT_DIESEL:
      Serial2.printf("get currentLimitDiesel");
      endCommand();
      break;
    case ValueType::LIMIT_SUPER:
      Serial2.printf("get currentLimitSuper");
      endCommand();
      break;
  }
}

int32_t Display::getReceivedValue()
{
  // TODO: wait for "received" event
  while (!valueReceived)
    delay(1);
  xSemaphoreTake(displayMutex, portMAX_DELAY);
  int32_t retValue = lastReceivedValue;
  lastReceivedValue = -1;
  valueReceived = false;
  xSemaphoreGive(displayMutex);
  return retValue;
}

// ==================================================================================
void displayReceiveTask(void* parameters)
{
  TRACE();
  Display* disp = (Display*)parameters;
  char* bufferPointer = disp->getReceiveBuffer();
  int32_t receiveIndex = 0;
  int32_t ffCount = 0;

  DEB_P("displayReceiveTask() running on core ");
  DEB_PL(xPortGetCoreID());

  while (1)
  {
    if (Serial2.available())
    {
      byte value  = Serial2.read();
      bufferPointer[receiveIndex] = value;
      receiveIndex++;
      //DEB_P(value, HEX);
      if (value == 0xFF)
      {
        ffCount++;
      }
      else
      {
        ffCount = 0;
      }

      if (ffCount == 3)
      {
        //DEB_PL();
        // end of data
        // handle event immeadiately

        switch (bufferPointer[0])
        {
          case 0x70:
            // Button release
            switch (bufferPointer[1])
            {
              case 0x4B:
                // Station key buttons
                // 70 4B 65 79 31 FF FF FF   pKey1
                // 70 4B 65 79 32 FF FF FF   pKey2
                // 70 4B 65 79 33 FF FF FF   pKey3
                // 70 4B 65 79 34 FF FF FF   pKey4
                // 70 4B 65 79 35 FF FF FF   pKey5
                DEB_PF("DISP: station key button %d pressed\n", bufferPointer[4] - '0');
                xSemaphoreTake(displayMutex, portMAX_DELAY);
                disp->setButtonEvent(ButtonEvent::KEY);
                disp->setButtonKey(bufferPointer[4] - '0');
                xSemaphoreGive(displayMutex);
                break;

              case 0x50:
                // invisible previous hotspot
                // 70 50 72 65 76 69 6F 75 73 FF FF FF   pPrevious
                DEB_PF("DISP: PREVIOUS hotspot pressed\n");
                disp->setButtonEvent(ButtonEvent::PREVIOUS);
                break;
              case 0x4E:
                // invisible previous hotspot
                // 70 4E 65 78 74 FF FF FF   pNext
                DEB_PF("DISP: PREVIOUS hotspot pressed\n");
                disp->setButtonEvent(ButtonEvent::NEXT);
                break;

              case 0x44:
                // invisible brighness hotspot
                // 70 44 61 72 6B 65 72 FF FF FF   pDarker
                DEB_PF("DISP: DARK hotspot pressed\n");
                disp->setButtonEvent(ButtonEvent::DARK);
                break;
              case 0x42:
                // invisible brighness hotspot
                // 70 42 72 69 67 68 74 65 72 FF FF FF   pBrighter
                DEB_PF("DISP: BRIGHT hotspot pressed\n");
                disp->setButtonEvent(ButtonEvent::BRIGHT);
                break;

              case 0x4C:
                // hotspot LEFT
                // 70 4C 65 66 74 FF FF FF   pLeft
                DEB_PF("DISP: LEFT hotspot pressed\n");
                disp->setButtonEvent(ButtonEvent::LEFT);
                break;
              case 0x4D:
                // headline middle button
                // 70 4D 69 64 64 6C 65 FF FF FF   pMiddle
                DEB_PF("DISP: MIDDLE hotspot pressed\n");
                disp->setButtonEvent(ButtonEvent::MIDDLE);
                break;
              case 0x52:
                // 70 52 69 67 68 74 FF FF FF   pRight
                DEB_PF("DISP: RIGHT hotspot pressed\n");
                disp->setButtonEvent(ButtonEvent::RIGHT);
                break;
              case 0x46:
                // 70 46 75 65 6C 4C 69 6D 69 74 73 FF FF FF   pFuelLimits
                DEB_PF("DISP: fuel price limits set\n");
                disp->setButtonEvent(ButtonEvent::LIMITS);
                break;

              default:
                break;
            }
            break;

          // receive limit values for fuel price
          case 0x71:
            // Simulator:
            // 71 7F 00 00 00 FF FF FF   127
            // 71 00 01 00 00 FF FF FF   256
            // Display:
            // 71 7F 00 FF FF FF   127
            // 71 00 01 FF FF FF   256
            DEB_PF("DISP: value %d\n", disp->lastReceivedValue);
            xSemaphoreTake(displayMutex, portMAX_DELAY);
            disp->lastReceivedValue = (int32_t)((int16_t)bufferPointer[1] + ((int16_t)bufferPointer[2] << 8));
            disp->valueReceived = true;
            xSemaphoreGive(displayMutex);
            break;

          default:
            break;
        }
        DEB_PF("DISP: stack %d\n", uxTaskGetStackHighWaterMark(NULL));
        ffCount = 0;
        receiveIndex = 0;
      }
    }
  }

  // emergency case:
  xSemaphoreGive(displayMutex);
  vTaskDelete(NULL);
}

// ==================================================================================
