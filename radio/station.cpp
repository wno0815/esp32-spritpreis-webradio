
#include "station.h"

// ============================================================================================================================

RadioStation::RadioStation() {}
/*
   full attribute constructor
   might be used for hard coded stations - e.g. DLF on station key 2:
      RadioStation dlf("Deutschlandfunk", "DLF", 2, "st01.dlf.de/dlf/01/128/mp3/stream.mp3");
*/
RadioStation::RadioStation(const char* stationName, const char* stationKeyName, uint32_t stationKey, const char* stationUrl) :
  uiName {stationName}, uiKeyName {stationKeyName}, uiKey {stationKey}, url {stationUrl} {};


/*
   set RadioStation attributes
*/
void RadioStation::setName(const char* stationName)
{
  uiName = stationName;
}
void RadioStation::setKeyName(const char* stationKeyName)
{
  uiKeyName = stationKeyName;
}
void RadioStation::setKey(uint32_t stationKey)
{
  uiKey = stationKey;
}
void RadioStation::setUrl(const char* stationUrl)
{
  url = stationUrl;
}

/*
   get RadioStation attributes
*/
const char* RadioStation::getName()
{
  return uiName;
}
const char* RadioStation::getKeyName()
{
  return uiKeyName;
}
uint32_t RadioStation::getKey()
{
  return uiKey;
}
const char* RadioStation::getUrl()
{
  return url;
}

void RadioStation::debugPrint()
{
  DEB_PF("%s | %s | %d | %s\n", uiName, uiKeyName, uiKey, url);
}

// ============================================================================================================================

RadioStations::RadioStations() {}

bool RadioStations::setDefaultStationIndex(int32_t index)
{
  defaultStationIndex = index;
  return true;
}

int32_t RadioStations::getDefaultStationIndex()
{
  return defaultStationIndex;
}

uint32_t RadioStations::getNumberOfStations()
{
  return numberOfStations;
}

bool RadioStations::createStationList(uint32_t number)
{
  TRACE();

  if (stationList != NULL)
  {
    DEB_PF("station list is not empty (%d elements). Deleting.\n", numberOfStations);
    delete stationList;
  }
  numberOfStations = number;
  stationList = new RadioStation[numberOfStations]();
  if (stationList == NULL)
  {
    DEB_PL("creation of station list failed");
    return false;
  }
  DEB_PF("station list created for %d elements\n", numberOfStations);
  return true;
}

bool RadioStations::setStation(uint32_t index, RadioStation& station)
{
  if (index >= numberOfStations)
  {
    DEB_PF("RadioStations::setStation: index %d is too large for list of %d elements\n", index, numberOfStations);
    return false;
  }
  stationList[index] = station;
  DEB_PF("station[%d] set to '%s'\n", index, stationList[index].getName());
  return true;
}

RadioStation& RadioStations::getStation(uint32_t index)
{
  if (index >= numberOfStations)
  {
    DEB_PF("RADIO: index %d is too large for list of %d elements\n", index, numberOfStations);
    return stationList[0];
  }
  return stationList[index];
}


RadioStation& RadioStations::operator[](uint32_t index)
{
  return getStation(index);
}

void RadioStations::debugPrint()
{
  DEB_PL("Radio station list:");
  DEB_PF("    number of stations: %d\n", numberOfStations);
  DEB_PF("    default station   : %d\n", defaultStationIndex);
  DEB_PL("    station list:");
  for (uint32_t count = 0; count < numberOfStations; count++)
  {
    DEB_PF("       [%3d]  ", count);
    stationList[count].debugPrint();
  }
}

// ============================================================================================================================

RadioStationKeys::RadioStationKeys() {}

bool RadioStationKeys::setStationIndex(uint8_t key, int32_t index)
{
  if (key >= numberOfStationKeys)
  {
    DEB_PF("[RadioStationKeys::setStationIndex] key %d is too large for array of %d elements\n", key, numberOfStationKeys);
    return false;
  }
  stationKeys[key] = index;
  return true;
}

int32_t RadioStationKeys::getStationIndex(uint8_t key)
{
  if (key >= numberOfStationKeys)
  {
    DEB_PF("[RadioStationKeys::getStationIndex] key %d is too large for array of %d elements\n", key, numberOfStationKeys);
    return -1;
  }
  return stationKeys[key];
}

void RadioStationKeys::debugPrint()
{
  DEB_PL("Station key list:");
  for (uint8_t count = 0; count < numberOfStationKeys; count++)
  {
    DEB_PF("    [%2d] = %3d\n", count, stationKeys[count]);
  }
}

void RadioStationKeys::debugPrint(RadioStations& stations)
{
  DEB_PL("Station key list:");
  for (uint8_t count = 0; count < numberOfStationKeys; count++)
  {
    DEB_PF("    [%2d] = %3d [%s]\n", count, stationKeys[count], stations[stationKeys[count]].getKeyName());
  }
}
