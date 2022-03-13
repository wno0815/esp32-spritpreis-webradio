
#include <ArduinoOTA.h>
#include <LITTLEFS.h>

#include "trace.h"
#include "config.h"

#include "player.h"
extern Player player;
extern Storage storage;
extern Display screen;


void setArduinoOTACallbacks()
{
  ArduinoOTA
  .onStart([]()
  {
    player.stop();
    String type;
    screen.selectPage(Pages::DOWNLOAD);
    screen.setProgress(0);
    if (ArduinoOTA.getCommand() == U_FLASH)
    {
      type = "sketch";
      screen.setType("Programm");
    }
    else // U_SPIFFS
    {
      type = "filesystem";
      screen.setType("Daten");
    }
    // stop filesystem - also in case of Sketch upload.
    LITTLEFS.end();
    DEB_PL("Start updating " + type);
  })
  .onEnd([]()
  {
    DEB_PL("\nEnd");
  })
  .onProgress([](unsigned int progress, unsigned int total)
  {
    byte percent = progress / (total / 100);
    screen.setProgress((byte)(percent > 100 ? 100 : percent));

    DEB_PF("Progress: %u%%\r", percent);
  })
  .onError([](ota_error_t error)
  {
    DEB_PF("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
    {
      screen.setType("Auth Failed");
      DEB_PL("Auth Failed");
    }
    else if (error == OTA_BEGIN_ERROR)
    {
      screen.setType("Begin Failed");
      DEB_PL("Begin Failed");
    }
    else if (error == OTA_CONNECT_ERROR)
    {
      screen.setType("Connect Failed");
      DEB_PL("Connect Failed");
    }
    else if (error == OTA_RECEIVE_ERROR)
    {
      screen.setType("Receive Failed");
      DEB_PL("Receive Failed");
    }
    else if (error == OTA_END_ERROR)
    {
      screen.setType("End Failed");
      DEB_PL("End Failed");      // omitting braces causes compiler waring if DEBUGGING is not defined
    }
  });
}



// VS1053 debug helper functions
//#define VS1053_VERBOSE
#if defined(VS1053_VERBOSE)
void vs1053_info(const char *info)                  // called from vs1053
{
  DEB_P("DEBUG:        ");
  DEB_PL(info);                           // debug infos
}
#endif

void vs1053_showstation(const char *info)           // called from vs1053
{
  DEB_P("STATION:      ");
  DEB_PL(info);                           // Show station name
}

#if 0
void vs1053_showstreamtitle(const char *info)       // called from vs1053
{
  DEB_P("STREAMTITLE:  ");
  DEB_PL(info);                           // Show title
}
#endif

#if defined(VS1053_VERBOSE)
void vs1053_showstreaminfo(const char *info)        // called from vs1053
{
  DEB_P("STREAMINFO:   ");
  DEB_PL(info);                           // Show streaminfo
}
#endif

#if defined(VS1053_VERBOSE)
void vs1053_eof_mp3(const char *info)               // called from vs1053
{
  DEB_P("vs1053_eof:   ");
  DEB_P(info);                             // end of mp3 file (filename)
}
#endif

#if defined(VS1053_VERBOSE)
void vs1053_bitrate(const char *br)                 // called from vs1053
{
  DEB_P("BITRATE:      ");
  DEB_PL(String(br) + "kBit/s");          // bitrate of current stream
}
#endif

#if defined(VS1053_VERBOSE)
void vs1053_commercial(const char *info)            // called from vs1053
{
  DEB_P("ADVERTISING:  ");
  DEB_PL(String(info) + "sec");           // info is the duration of advertising
}
#endif

#if defined(VS1053_VERBOSE)
void vs1053_icyurl(const char *info)                // called from vs1053
{
  DEB_P("Homepage:     ");
  DEB_PL(info);                           // info contains the URL
}
#endif

#if defined(VS1053_VERBOSE)
void vs1053_eof_speech(const char *info)            // called from vs1053
{
  DEB_P("end of speech:");
  DEB_PL(info);
}
#endif

#if defined(VS1053_VERBOSE)
void vs1053_lasthost(const char *info)              // really connected URL
{
  DEB_P("lastURL:      ");
  DEB_PL(info);
}
#endif


bool nextionUpdate()
{
  TRACE();

  size_t updateSize = storage.openUpdateFile();
  uint8_t buffer[segmentSize];
  size_t segments;
  size_t rest;
  size_t bytesRead = 0;
  size_t bytesSent = 0;

  uint8_t ch;

  if (updateSize == 0)
  {
    DEB_PL("cannot update due to missing TFT file");
    return false;
  }

  player.stop();
  screen.selectPage(Pages::PLAYER);
  screen.setStation("Nextion Update");
  screen.setTitle("");
  screen.deactivateKeys(numberOfStationKeys);
  screen.off();
  theClock.off();
  delay(1000);
  DEB_PL("system is off");

  // change baud rate
  DEB_PF("change baud rate to %zubps\n", uploadSpeed);
  Serial2.printf("baud=%zu", uploadSpeed);
  screen.endCommand();
  delay(1000);
  Serial2.begin(uploadSpeed);
  delay(100);

  // TODO: avoid OTA upload

  segments = updateSize / segmentSize;
  rest = updateSize % segmentSize;
  DEB_PF("update size is %zu bytes (%zu blocks / %zu rest)\n", updateSize, segments, rest);

  DEB_P("empty command ");
  screen.endCommand();
  // wait for reply
  for (int i = 0; i < 100; i++)
  {
    if (Serial2.available())
    {
      ch = Serial2.read();
      if (ch != 0xFF)
      {
        DEB_P(ch);
      }
    }
    delay (20) ;
  }
  DEB_PL(" ...done");

  // DO DOWNLOAD
  // Start upload
  DEB_PF("START upload: whmi-wri %zu,%zu,0\n", updateSize, uploadSpeed);
  Serial2.printf("whmi-wri %zu,%zu,0", updateSize, uploadSpeed);
  screen.endCommand();
  while (!Serial2.available())
  {
    delay(20) ;
  }
  ch = Serial2.read();
  if (ch != 0x05)
  {
    DEB_PL("start failed");
    goto errorEnd;
  }

  for (size_t count = 0; count < segments; count++)
  {
    bytesRead = storage.readUpdateFile(segmentSize, buffer);
    if (bytesRead == segmentSize)
    {
      // transmit to display
      Serial2.write(buffer, segmentSize) ;
      // wait for and read ACK
      while (!Serial2.available())
      {
        delay(20) ;
      }
      ch = (char)Serial2.read();
      if (ch != 0x05)
      {
        goto errorEnd;
      }
      // show progress
      bytesSent += segmentSize;
      DEB_PF(" %7zu (%3zu%%) sent\r", bytesSent, bytesSent * 100 / updateSize);
    }
    else
    {
      DEB_PF("\nread error: wanted %zu, got %zu at segment %zu\n", segmentSize, bytesRead, count);
      goto errorEnd;
    }
  }
  if (rest)
  {
    bytesRead = storage.readUpdateFile(rest, buffer);
    if (bytesRead == rest)
    {
      // transmit to display
      Serial2.write(buffer, rest) ;
      // wait for and read ACK
      while (!Serial2.available())
      {
        delay(20) ;
      }
      ch = (char)Serial2.read();
      if (ch != 0x05)
      {
        goto errorEnd;
      }
      // show progress
      bytesSent += rest;
      DEB_PF(" %7zu (%3zu%%) sent\r", bytesSent, bytesSent * 100 / updateSize);
    }
    else
    {
      DEB_PF("\nread error: wanted %zu, got %zu at rest\n", rest, bytesRead);
    }
    DEB_PL();
  }
errorEnd:
  DEB_PL();
  DEB_PL("update finished");
  storage.closeUpdateFile();
  Serial2.begin(9600);
  delay(1000);

  return true;
}


void populateStationKeys()
{
  for (uint8_t count = 1; count < numberOfStationKeys; count++)
  {
    screen.setKeyText(count, stations[keys.getStationIndex(count)].getKeyName());
  }
}

void updateStationKeys(int32_t currentStationIndex)
{
  for (uint8_t count = 1; count < numberOfStationKeys; count++)
  {
    if (keys.getStationIndex(count) == currentStationIndex)
    {
      screen.activateKey(count, true);
    }
    else
    {
      screen.activateKey(count, false);
    }
  }
}

void initPlayerPage()
{
  if (screen.getCurrentPage() == Pages::PLAYER)
  {
    int32_t currentStationIndex = player.getCurrentStationIndex();
    populateStationKeys();
    screen.setStation(stations[currentStationIndex].getName());
    updateStationKeys(currentStationIndex);
    screen.setTitle(player.getTitleText());
  }
}

void initClockPage()
{
  if (screen.getCurrentPage() == Pages::CLOCK)
  {
    theClock.forceUpdate();
    screen.setClockTime(theClock.getHour(), theClock.getMinute(), theClock.getSecond(), theClock.getDate(), theClock.getWeekday());
  }
}

void initFuelPage(bool setName)
{
  if (screen.getCurrentPage() == Pages::FUEL)
  {
    FuelStation station = fuels[fuels.getCurrentStationIndex()];
    if (setName)
      screen.setFuelData(station.getUiName(), station.getPrice(FuelType::DIESEL), station.getPrice(FuelType::SUPER), station.isOpen());
    else
      screen.setFuelPrices(station.getPrice(FuelType::DIESEL), station.getPrice(FuelType::SUPER), station.isOpen());

    screen.setFuelAlarmDiesel(station.getAlarm(FuelType::DIESEL));
    screen.setFuelAlarmSuper(station.getAlarm(FuelType::SUPER));
  }
}
