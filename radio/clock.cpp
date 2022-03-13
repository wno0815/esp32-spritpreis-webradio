
#include "trace.h"

#include "clock.h"
void clockTask(void* parameters);
time_t clockNow;    // this is the epoch
tm clockTim;        // the structure tm holds time information in a more convient way
const char* weekdayText[] = { "Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Sonnabend" };



Clock::Clock() {};

void Clock::begin()
{
  TRACE();

  configTime(0, 0, ntpServer);          // 0, 0 because we will use TZ in the next line
  setenv("TZ", ntpTimeszone, 1);        // Set environment variable with your time zone
  tzset();

  xTaskCreate(    clockTask,            /* Task function. */
                  "ClockTask",          /* String with name of task. */
                  4096,                 /* Stack size in bytes. */
                  (void*)this,          /* Parameter passed as input of the task: This Display object*/
                  1,                    /* Priority of the task. */
                  &clockTaskHandle);    /* Task handle. */
  delay(100);
  DEB_PF("CLOCK: clock task created\n");
}

void Clock::forceUpdate()
{
  // DEB_PL("CLOCK: force clock update (call ntp)");
  time(&clockNow);                      // read the current time
  localtime_r(&clockNow, &clockTim);    // update the structure tm with the current time
}

uint8_t Clock::getHour()
{
  return clockTim.tm_hour;
}
uint8_t Clock::getMinute()
{
  return clockTim.tm_min;
}
uint8_t Clock::getSecond()
{
  return clockTim.tm_sec;
}

char* Clock::getDate()
{
  sprintf(dateText, "%d.%d.%d", clockTim.tm_mday, clockTim.tm_mon + 1, clockTim.tm_year + 1900);
  return dateText;
}

const char* Clock::getWeekday()
{
  return weekdayText[clockTim.tm_wday];
}

bool Clock::secondEventStatus()
{
  if (secondEvent == true)
  {
    secondEvent = false;
    return true;
  }
  return false;
}

bool Clock::ntpEventStatus()
{
  if (ntpEvent == true)
  {
    ntpEvent = false;
    return true;
  }
  return false;
}

bool Clock::fuelEventStatus()
{
  if (fuelEvent == true)
  {
    fuelEvent = false;
    resetFuelEventInterval = true;
    return true;
  }
  return false;
}

void Clock::setSecondEvent()
{
  secondEvent = true;
}

void Clock::setNtpEvent()
{
  ntpEvent = true;
}

void Clock::setFuelEvent()
{
  fuelEvent = true;
}


void Clock::off()
{
  if (clockTaskHandle != NULL)
  {
    // responses from display are handled in update
    vTaskDelete(clockTaskHandle);
    clockTaskHandle = NULL;
    secondEvent = false;
    ntpEvent = false;
    fuelEvent = false;
  }
  statusOn = false;
  DEB_PL("CLOCK: clock is off");
}

bool Clock::isOn()
{
  return statusOn;
}

// ==================================================================================
void clockTask(void* parameters)
{
  Clock* theClock = (Clock*)parameters;
  unsigned long currentTime = millis();
  unsigned long lastTimeRead = currentTime;
  unsigned long lastTimeNtpRead = currentTime;
  unsigned long lastTimeFuelRead = currentTime;
  unsigned long realNtpUpdateInterval = ::ntpUpdateInterval * 1000UL;
  unsigned long realFuelUpdateInterval = ::fuelUpdateInterval * 1000UL;

  TRACE();
  DEB_P("CLOCK_TASK: clockTask() running on core ");
  DEB_PL(xPortGetCoreID());
  DEB_PF("CLOCK_TASK: ntp server is %s; update interval %lu\n", ::ntpServer, ::ntpUpdateInterval);
  DEB_PF("CLOCK_TASK: Tankerkoenig update interval %lu\n", ::fuelUpdateInterval);

  // initial
  theClock->setSecondEvent();
  theClock->setNtpEvent();
  theClock->setFuelEvent();

  while (1)
  {
    currentTime = millis();

    if (currentTime - lastTimeRead > 1000UL)
    {
      // tick
      lastTimeRead = currentTime;
      theClock->setSecondEvent();
      clockNow++;
      localtime_r(&clockNow, &clockTim);   // update the structure tm with the current time

      // ntp update
      if (currentTime - lastTimeNtpRead > realNtpUpdateInterval)
      {
        lastTimeNtpRead = currentTime;
        theClock->setNtpEvent();
        //DEB_PF("CLOCK_TASK : stack %d\n", uxTaskGetStackHighWaterMark(NULL));
      }

      // Tankerkoenig fuel price update
      if (theClock->resetFuelEventInterval)
      {
        // if an already set FuelEvent is recognized later (due to OFF state)
        // the update interval shall be retriggered
        theClock->resetFuelEventInterval = false;
        lastTimeFuelRead = currentTime;
      }

      if (currentTime - lastTimeFuelRead > realFuelUpdateInterval)
      {
        lastTimeFuelRead = currentTime;
        theClock->setFuelEvent();
      }
    }

    delay(6);
  }

  // emergency case:
  vTaskDelete(NULL);
}

// ==================================================================================
