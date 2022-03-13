
#include <ArduinoOTA.h>

#include "trace.h"

const char* radioVersion = "0.99.0";

#include "config.h"
#include "station.h"
#include "network.h"
#include "storage.h"
#include "player.h"
#include "display.h"
#include "clock.h"
#include "encoder.h"


Storage storage;
RadioStations stations;
RadioStationKeys keys;
Networks networks;
Player player;
Display screen;
Clock theClock;
Encoder encoder;
FuelStations fuels;

bool isConnected = false;
bool isOn = true;
int32_t offBrightness;
Pages lastPageBeforeFuelAlarm = startPage;
bool fuelAlarm = false;
bool lastFuelAlarm = false;
bool wakeUpByFuelAlarm = false;


void setup()
{
  DEB_B(115200);
  delay(1000);
  DEB_H();
  TRACE();

  screen.begin();
  screen.debug("ESP32 radio ");
  screen.debug(radioVersion, true);
  DEB_P("setup() running on core ");
  DEB_PL(xPortGetCoreID());

  if (!storage.begin())
  {
    DEB_PL("storage begin failed");
  }
  else
  {
    // list content
    DEB_PL("Listing root directory");
    screen.debug("file system:", false);
    screen.debug("  total size: ", false);
    screen.debug(LITTLEFS.totalBytes(), true);
    screen.debug("  used bytes: ", false);
    screen.debug(LITTLEFS.usedBytes(), true);

    File root = LITTLEFS.open("/");
    File file = root.openNextFile();
    while (file)
    {
      screen.debug("  ", false);
      screen.debug(file.name(), true);
      screen.debug(" [", true);
      screen.debug(file.size(), true);
      screen.debug("]", true);
      file.close();
      file = root.openNextFile();
    }
    root.close();
  }

  // get data and connect to network
  screen.debug("get data");
  storage.getStationList(stations, keys);
  screen.debug("  stations: ");
  screen.debug(stations.getNumberOfStations(), true);
  int32_t currentStationIndex = storage.getCurrentStationIndex(stations);
  player.setCurrentStationIndex(currentStationIndex, stations.getNumberOfStations());   // save station (needed if not starting with Player screen)
  screen.debug("  current : ");
  screen.debug(currentStationIndex, true);
  int32_t currentBrightness = storage.getCurrentBrightness();
  screen.debug("  display brightness : ");
  screen.debug(currentBrightness, true);
  storage.getStationList(fuels);
  storage.getCurrentFuelPriceLimit(FuelType::DIESEL, fuels);
  storage.getCurrentFuelPriceLimit(FuelType::SUPER, fuels);
  screen.setFuelLimits(fuels.getLimit(FuelType::DIESEL), fuels.getLimit(FuelType::SUPER));

  screen.debug("  fuel stations: ");
  screen.debug(fuels.getNumberOfStations(), true);

  storage.getNetworkList(networks);
  networks.createAvailableNetworkList();
  networks.matchNetwork();
  screen.debug("  networks: ");
  screen.debug(networks.getNumberOfAvailableNetworks(), true);

  screen.debug("connect to network");
  isConnected = networks.connectNetwork();
  if (isConnected)
  {
    screen.debug("  ");
    screen.debug(networks.getCurrentName(), true);
    screen.debug("  ", true);
    screen.debug(networks.getCurrentIP(), true);
  }
  else
  {
    screen.debug("  failed");
  }

  // prepare for OTA updates
  setArduinoOTACallbacks();
  if (isConnected)
  {
    ArduinoOTA.begin();
  }

  screen.debug("components: ");
  // setup time
  screen.debug(" clock", true);
  theClock.begin();
  // encoder
  screen.debug(", encoder", true);
  encoder.begin(screen);
  // VS 1053 modules
  screen.debug(", player", true);
  player.begin(stations, keys);

  screen.debug("Initialisation complete");
  delay(2000);
  screen.setBrightness(currentBrightness);

  // for test: Let's see the result nicely printed
  DEB_H();
  DEB_H();
  stations.debugPrint();
  keys.debugPrint(stations);
  networks.debugPrint();
  fuels.debugPrint();
  DEB_H();
  DEB_H();

  // start with first page as selected in config.h
  switch (startPage)
  {
    case Pages::DEBUG:
      // not recommended - stay on DEBUG screen
      break;
    case Pages::PLAYER:
      if (stations.getNumberOfStations() && isConnected)
      {
        player.playStation(currentStationIndex);
        screen.selectPage(Pages::PLAYER);
        initPlayerPage();
        isOn = true;
      }
      break;
    case Pages::FUEL:
      screen.selectPage(Pages::FUEL);
      initFuelPage(true);
      isOn = true;
      break;
    case Pages::CLOCK:
      // CLOCK as start page always means "OFF"
      screen.selectPage(Pages::CLOCK);
      offBrightness = screen.setBrightness(brightnessWhenOff);
      initClockPage();
      isOn = false;
      break;
    case Pages::DOWNLOAD:
      break;
  }

  DEB_H();
}


void loop()
{
  static bool executedOnce = false;

  if (!executedOnce)
  {
    DEB_P("loop() running on core ");
    DEB_PL(xPortGetCoreID());

    executedOnce = true;
  }


  isConnected = networks.checkNetwork();

  if (isConnected)
  {
    ArduinoOTA.handle();
    player.run();

    // handle player updates
    if (screen.getCurrentPage() == Pages::PLAYER)
    {
      if (player.hasStationChanged())
      {
        // update station name
        int32_t currentStation = player.getCurrentStationIndex();
        if (!player.isPlaying())
        {
          screen.setStation("RADIO");
          screen.setTitle("");

          // all station keys off
          screen.deactivateKeys(numberOfStationKeys);
        }
        else
        {
          storage.putCurrentStationIndex(currentStation);
          screen.setStation(stations[currentStation].getName());
          screen.setTitle(player.getTitleText());
          updateStationKeys(currentStation);
        }
      }
    }

    // handle encoder and buttons from display
    switch (encoder.eventStatus())
    {
      case EncoderEvent::CLICK:
        if (isOn)
        {
          if (player.isPlaying())
          {
            // --> "off"
            player.stop();
          }
          if (fuelAlarm)
          {
            // When switched off while Alarm is active:
            // Need to correct the "page active before" information -
            // when the alarm disappears it shall use the right page
            lastPageBeforeFuelAlarm = Pages::CLOCK;
          }
          if (screen.getCurrentPage() != Pages::CLOCK)
          {
            screen.selectPage(Pages::CLOCK);
            initClockPage();
          }
          offBrightness = screen.setBrightness(brightnessWhenOff);
          isOn = false;
        }
        else
        {
          // --> "on" (always switches to player)
          player.playStation(player.getCurrentStationIndex());
          screen.selectPage(Pages::PLAYER);
          initPlayerPage();
          screen.setBrightness(offBrightness);
          isOn = true;
        }
        wakeUpByFuelAlarm = false;    // reset due to user operation of the device (do not switch off if alarm goes away)
        break;
      case EncoderEvent::LONGCLICK:
        DEB_PL("EncoderEvent::LONGCLICK");
        // prepare for future UI extensions
        //        if (isOn)
        //        {
        //          switch (screen.getCurrentPage())
        //          {
        //            case Pages::DEBUG:
        //              break;
        //            case Pages::PLAYER:
        //              break;
        //            case Pages::FUEL:
        //              break;
        //            case Pages::CLOCK:
        //              break;
        //          }
        //        }
        //        else
        //        {
        //          switch (screen.getCurrentPage())
        //          {
        //            case Pages::DEBUG:
        //              break;
        //            case Pages::PLAYER:
        //              break;
        //            case Pages::FUEL:
        //              break;
        //            case Pages::CLOCK:
        //              break;
        //            case Pages::DOWNLOAD:
        //              break;
        //          }
        //        }
        break;
      case EncoderEvent::TURN_LEFT:
        DEB_PF("EncoderEvent::TURN_LEFT by %d ticks\n", encoder.getTicks());
        screen.setButtonEvent(ButtonEvent::PREVIOUS);
        break;
      case EncoderEvent::TURN_RIGHT:
        DEB_PF("EncoderEvent::TURN_RIGHT by %d ticks\n", encoder.getTicks());
        screen.setButtonEvent(ButtonEvent::NEXT);
        break;
      case EncoderEvent::NONE:
        break;
    }

    // handle buttons from display
    switch (screen.buttonEventStatus())
    {
      case ButtonEvent::KEY:
        player.playKey(screen.getButtonKey());
        break;
      case ButtonEvent::PREVIOUS:
        switch (screen.getCurrentPage())
        {
          case Pages::DEBUG:
            break;
          case Pages::PLAYER:
            player.playNextPrevious(false);
            break;
          case Pages::CLOCK:
            break;
          case Pages::FUEL:
            fuels.selectNextPrevious(false);
            initFuelPage(true);
            break;
          case Pages::DOWNLOAD:
            break;
        }
        break;
      case ButtonEvent::NEXT:
        switch (screen.getCurrentPage())
        {
          case Pages::DEBUG:
            break;
          case Pages::PLAYER:
            player.playNextPrevious(true);
            break;
          case Pages::CLOCK:
            break;
          case Pages::FUEL:
            fuels.selectNextPrevious(true);
            initFuelPage(true);
            break;
          case Pages::DOWNLOAD:
            break;
        }
        break;
      case ButtonEvent::DARK:
        storage.putCurrentBrightness(screen.adjustBrightness(false));
        break;
      case ButtonEvent::BRIGHT:
        storage.putCurrentBrightness(screen.adjustBrightness(true));
        break;
      case ButtonEvent::LEFT:
        switch (screen.getCurrentPage())
        {
          case Pages::DEBUG:
            screen.selectPage(Pages::CLOCK);
            initClockPage();
            break;
          case Pages::PLAYER:
            screen.selectPage(Pages::DEBUG);
            break;
          case Pages::FUEL:
            screen.selectPage(Pages::PLAYER);
            initPlayerPage();
            break;
          case Pages::CLOCK:
            screen.selectPage(Pages::FUEL);
            initFuelPage(true);
            break;
          case Pages::DOWNLOAD:
            break;
        }
        wakeUpByFuelAlarm = false;    // reset due to user operation of the device (do not switch off if alarm goes away)
        break;
      case ButtonEvent::RIGHT:
        switch (screen.getCurrentPage())
        {
          case Pages::DEBUG:
            screen.selectPage(Pages::PLAYER);
            initPlayerPage();
            break;
          case Pages::PLAYER:
            screen.selectPage(Pages::FUEL);
            initFuelPage(true);
            break;
          case Pages::FUEL:
            screen.selectPage(Pages::CLOCK);
            initClockPage();
            break;
          case Pages::CLOCK:
            screen.selectPage(Pages::DEBUG);
            break;
          case Pages::DOWNLOAD:
            break;
        }
        wakeUpByFuelAlarm = false;    // reset due to user operation of the device (do not switch off if alarm goes away)
        break;
      case ButtonEvent::MIDDLE:
        if (nextionUpdate())
          ESP.restart();
        break;

      case ButtonEvent::LIMITS:
        // new fuel price limits have been set.
        // retrieve and store
        {
          int32_t limitDiesel;
          int32_t limitSuper;

          // TODO:
          // This code waits in getReceivedValue() for the value transferred from the display
          // Correct implementation would be some kind of state machine - but this would have impact
          // on whole UI processing.
          // Since SerialPort for display is set to 38400bps - we can wait a moment
          screen.requestValue(ValueType::LIMIT_DIESEL);
          limitDiesel = screen.getReceivedValue();
          screen.requestValue(ValueType::LIMIT_SUPER);
          limitSuper = screen.getReceivedValue();

          fuels.setLimit(FuelType::DIESEL, limitDiesel);
          fuels.setLimit(FuelType::SUPER, limitSuper);
          storage.putCurrentFuelPriceLimit(FuelType::DIESEL, limitDiesel);
          storage.putCurrentFuelPriceLimit(FuelType::SUPER, limitSuper);
          screen.setFuelLimits(limitDiesel, limitSuper);

          fuels.checkLimits();
          initFuelPage(true);
        }
        break;

      case ButtonEvent::NONE:
        break;
    }

    // handle clock
    if (theClock.secondEventStatus())
    {
      screen.incrementClockSecond();
    }

    if (theClock.ntpEventStatus())
    {
      theClock.forceUpdate();
      if (screen.getCurrentPage() == Pages::CLOCK)
      {
        screen.setClockTime(theClock.getHour(), theClock.getMinute(), theClock.getSecond(), theClock.getDate(), theClock.getWeekday());
      }
    }

    if (isOn || enableFuelPriceScanWhileOff)
    {
      if (theClock.fuelEventStatus())
      {
        uint8_t currentHour = theClock.getHour();
        DEB_PF("[%2.2d:%2.2d:%2.2d] ", currentHour, theClock.getMinute(), theClock.getSecond());
        if ( (currentHour >= fuelScanStartHour) && (currentHour < fuelScanEndHour))
        {
          // get data from Tankerkoenig
          fuels.updatePrices();
        }
        else
        {
          // virtually close all stations if outside of scan time
          fuels.updatePrices(true);
        }

        // checkLimits might change the current station
        fuelAlarm = fuels.checkLimits();
        if (fuelAlarm != lastFuelAlarm)
        {
          if (fuelAlarm)
          {
            // alarm occured
            // change page and show
            lastPageBeforeFuelAlarm = screen.getCurrentPage();
            screen.selectPage(Pages::FUEL);
            if (enableFuelPriceScanWhileOff && !isOn)
            {
              // need to switch on again
              screen.setBrightness(offBrightness);
              isOn = true;
              wakeUpByFuelAlarm = true;
            }
            // announce by gong and speech
            player.playFile();
            if (enableSpeechOutput)
            {
              player.setSpeechPending(true);
            }
          }
          else
          {
            // alarm has gone
            switch (lastPageBeforeFuelAlarm)
            {
              case Pages::DEBUG:
                screen.selectPage(Pages::DEBUG);
                break;
              case Pages::PLAYER:
                screen.selectPage(Pages::PLAYER);
                initPlayerPage();
                break;
              case Pages::FUEL:
                screen.selectPage(Pages::FUEL);
                initFuelPage(true);
                break;
              case Pages::CLOCK:
                if (wakeUpByFuelAlarm)  // has been switched on by alarm function
                {
                  // do nothing - switch to clock scree will be done by switching off again
                }
                else
                {
                  screen.selectPage(Pages::CLOCK);
                  initClockPage();
                }
                break;
              case Pages::DOWNLOAD:
                break;
            }
            if (wakeUpByFuelAlarm)  // has been switched on by alarm function
            {
              // switch off again
              wakeUpByFuelAlarm = false;
              encoder.setEncoderEvent(EncoderEvent::CLICK);
            }
          }
          lastFuelAlarm = fuelAlarm;
        }
        // need to re-write station name in case of alarm also if FUEL page is active - it might have changed
        initFuelPage(fuelAlarm || (!(screen.getCurrentPage() == Pages::FUEL)));
      }
    }


    // TEST - simulate input devices
    if (Serial.available())
    {
      byte value = Serial.read();
      switch (value)
      {
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
          player.playKey(value - '0');
          break;
        case '<':
          player.playNextPrevious(false);
          break;
        case '>':
          player.playNextPrevious(true);
          break;
        case '+':
          player.changeVolume(true);
          break;
        case '-':
          player.changeVolume(false);
          break;
        case 'd':
          storage.putCurrentBrightness(screen.adjustBrightness(false));
          break;
        case 'b':
          storage.putCurrentBrightness(screen.adjustBrightness(true));
          break;
        case 'o':
          encoder.setEncoderEvent(EncoderEvent::CLICK);
          break;
        case 'f':
          theClock.setFuelEvent();
          break;
        case 'g':
          player.playFile(gongFile);
          break;
        case 's':
          player.playSpeech("Diesel in Holle jetzt 1 Euro 48 9");
          break;
        case 'u':
          if (nextionUpdate())
            ESP.restart();
          break;
        default:
          // ignore
          break;
      }
    }
  }
}

// Streamtitle is special: may change during playing.
// Capture event from VS1053 library
void vs1053_showstreamtitle(const char *info)       // called from vs1053
{
  DEB_P("stream title:  ");
  DEB_PL(info);                           // Show title
  player.setTitleText(info);
  screen.setTitle(info);
}

// Playing MP3 needs attention for end of title
void vs1053_eof_mp3(const char *info)               // called from vs1053
{
  DEB_P("mp3 end of file: ");
  DEB_PL(info);                           // Show info
  if (player.getSpeechPending())
  {
    bool super = fuels[fuels.getCurrentStationIndex()].getAlarm(FuelType::SUPER);
    bool diesel = fuels[fuels.getCurrentStationIndex()].getAlarm(FuelType::DIESEL);
    player.setSpeechPending(false);
    if (super)
    {
      player.playSpeech(fuels.getAlarmText(FuelType::SUPER));
      if (diesel)
      {
        // there will be a second speech
        player.setSpeechPending(true);
      }
    }
    else if (diesel)
    {
      player.playSpeech(fuels.getAlarmText(FuelType::DIESEL));
    }
  }
  else
  {
    player.resumeAfterFileOrSpeech();
  }
}

void vs1053_eof_speech(const char *info)            // called from vs1053
{
  DEB_P("end of speech: ");
  DEB_PL(info);

  if (player.getSpeechPending())
  {
    player.setSpeechPending(false);
    // is DIESEL still below limit?
    if (fuels[fuels.getCurrentStationIndex()].getAlarm(FuelType::DIESEL))
    {
      player.playSpeech(fuels.getAlarmText(FuelType::DIESEL));    // Super
    }
  }
  else
  {
    player.resumeAfterFileOrSpeech();
  }
}
