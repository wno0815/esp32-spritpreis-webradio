
/*
   Handles all storage

   LITTLEFS
    - network credentials in file /network.json
    - station list in file /stations.json
   NVS (Preferences)
    - current station playing

*/

#define ARDUINOJSON_USE_LONG_LONG 0
#define ARDUINOJSON_USE_DOUBLE 0
#include <ArduinoJson.h>


#include "storage.h"

bool Storage::begin()
{
  TRACE();

  // check if filesystem is available
  if (!LITTLEFS.begin(false))
  {
    DEB_PL("LITTLEFS mount failed");
    DEB_PL("no filesystem found; try to format");
    // failed, try again with format
    if (!LITTLEFS.begin(true))
    {
      DEB_PL("LITTLEFS format and mount failed");
      return false;
    }
    else
    {
      DEB_PL("filesystem formatted.");
    }
  }

  DEB_PL("LITTLEFS info:");
  DEB_PF("  total size : %zu\n",   LITTLEFS.totalBytes());
  DEB_PF("  used bytes : %zu\n\n", LITTLEFS.usedBytes());

  return true;
}

bool Storage::getStationList(RadioStations& stationList, RadioStationKeys& keyList)
{
  TRACE();

  // station list file
  File stations = LITTLEFS.open(radioStationsFile);
  if (stations && !stations.isDirectory())
  {
    DEB_PL("stations list file open");
    size_t  siz = stations.size();
    char* buf = new char[siz + 1];
    if (buf)
    {
      stations.readBytes(buf, siz);
      stations.close();
      buf[siz] = 0;
      DEB_PF("  %zu bytes read\n", siz);
      DEB_PL("stations list file closed");

      StaticJsonDocument<jsonRadioStationListDocSize> doc;
      DeserializationError error = deserializeJson(doc, buf, siz);
      if (error)
      {
        DEB_P("deserializeJson() failed: ");
        DEB_PL(error.c_str());
        return false;
      }

      JsonArray stationListArray = doc["StationList"].as<JsonArray>();
      stationList.createStationList(stationListArray.size());
      stationList.setDefaultStationIndex(doc["DefaultStationIndex"]);

      int count = 0;
      RadioStation current;
      for (JsonObject StationList_item : stationListArray)
      {
        current.setName(StationList_item["name"]);
        current.setKeyName(StationList_item["key"]);
        current.setKey(StationList_item["mem"]);
        current.setUrl(StationList_item["url"]);
        keyList.setStationIndex(current.getKey(), count);            // station list index for station memory key; 0 not used.
        stationList.setStation(count, current);
        count++;
      }
    }
    else
    {
      stations.close();
      DEB_PF("storing %zu bytes from file failed\n", siz);
    }
  }
  else
  {
    DEB_PL("station list file not found");
  }

  return true;
}

int32_t Storage::getCurrentStationIndex(RadioStations& stationList)
{
  TRACE();

  int32_t retValue = 0;

  if (!prefs.begin(settingsNamespace, false))
  {
    DEB_PL("open preferences namespace 'settings' failed");
    return -1;
  }
  else
  {
    DEB_PL("namespace 'settings' is initialized");
    retValue = prefs.getInt(settingsKeyCurrentStation, -1);
    if (retValue == -1)
    {
      retValue = stationList.getDefaultStationIndex();
      DEB_PF("write initial station index: %d\n", retValue);
      int32_t retError = prefs.putInt(settingsKeyCurrentStation, retValue);
      if (retError == 0)
      {
        DEB_PF("write initial current station index %d failed\n", retValue);
        prefs.end();
        return -2;
      }
    }
    else
    {
      DEB_PF("current station index: %d\n", retValue);
    }
    prefs.end();
  }
  // save value internally to avoid multiple writes of same value
  lastCurrentStationIndex = retValue;

  return retValue;
}


bool Storage::putCurrentStationIndex(int32_t index)
{
  TRACE();

  if (!prefs.begin(settingsNamespace, false))
  {
    DEB_PL("open preferences namespace 'settings' failed");
    return false;
  }
  else
  {
    if (index == lastCurrentStationIndex)
    {
      // nothing to do: value is already there
      prefs.end();
      return true;
    }
    DEB_PL("namespace 'settings' is initialized");
    size_t retValue = prefs.putInt(settingsKeyCurrentStation, index);
    if (retValue == 0)
    {
      DEB_PF("write current station index %d failed\n", index);
      return false;
    }
    DEB_PF("current station index %d written\n", index);
    prefs.end();
  }
  // save value internally to avoid multiple writes of same value
  lastCurrentStationIndex = index;

  return true;
}

int32_t Storage::getCurrentBrightness()
{
  TRACE();

  int32_t retValue = 0;

  if (!prefs.begin(settingsNamespace, false))
  {
    DEB_PL("open preferences namespace 'settings' failed");
    return -1;
  }
  else
  {
    DEB_PL("namespace 'settings' is initialized");
    retValue = prefs.getInt(settingsKeyBrightness, -1);
    if (retValue == -1)
    {
      retValue = 100;
      DEB_PF("write initial brightness: %d\n", retValue);
      int32_t retError = prefs.putInt(settingsKeyBrightness, retValue);
      if (retError == 0)
      {
        DEB_PF("write initial brightness %d failed\n", retValue);
        prefs.end();
        return -2;
      }
    }
    else
    {
      DEB_PF("current brightness: %d\n", retValue);
    }
    prefs.end();
  }
  // save value internally to avoid multiple writes of same value
  lastBrightness = retValue;

  return retValue;
}


bool Storage::putCurrentBrightness(int32_t value)
{
  TRACE();

  if (!prefs.begin(settingsNamespace, false))
  {
    DEB_PL("open preferences namespace 'settings' failed");
    return false;
  }
  else
  {
    if (value == lastBrightness)
    {
      // nothing to do: value is already there
      prefs.end();
      return true;
    }
    DEB_PL("namespace 'settings' is initialized");
    size_t retValue = prefs.putInt(settingsKeyBrightness, value);
    if (retValue == 0)
    {
      DEB_PF("write current brightness %d failed\n", value);
      return false;
    }
    DEB_PF("current brightness %d written\n", value);
    prefs.end();
  }
  // save value internally to avoid multiple writes of same value
  lastBrightness = value;

  return true;
}


bool Storage::getNetworkList(Networks& networkList)
{
  TRACE();

  // network list file
  File nets = LITTLEFS.open(networkCredentialsFile);
  if (nets && !nets.isDirectory())
  {
    DEB_PL("network list file open");

    size_t  siz = nets.size();
    // ATTENTION:
    // do never delete this buffer.
    // ArduinoJson uses it by just setting pointers
    char* buf = new char[siz + 1];
    if (buf)
    {
      nets.readBytes(buf, siz);
      nets.close();
      buf[siz] = 0;
      DEB_PF("  %zu bytes read\n", siz);
      DEB_PL("network list file closed");

      StaticJsonDocument<jsonNetworkListDocSize> doc;
      DeserializationError error = deserializeJson(doc, buf, siz);
      if (error)
      {
        DEB_P("deserializeJson() failed: ");
        DEB_PL(error.c_str());
        return false;
      }

      JsonArray networkListArray = doc["NetworkList"].as<JsonArray>();
      networkList.createNetworkList(networkListArray.size());

      int count = 0;
      Network current;
      for (JsonObject NetworkList_item : networkListArray)
      {
        current.setSSID(NetworkList_item["ssid"]);
        current.setPassword(NetworkList_item["password"]);
        networkList.setNetwork(count, current);
        count++;
      }
    }
    else
    {
      nets.close();
      DEB_PF("storing %zu bytes from file failed\n", siz);
    }
  }
  else
  {
    DEB_PL("network list file not found");
  }

  return true;
}

size_t Storage::openUpdateFile()
{
  tftFile = LITTLEFS.open(nextionTftFile);

  if (tftFile && !tftFile.isDirectory())
  {
    DEB_PL("tftFile open");
    tftFileSize = tftFile.size();
  }
  else
  {
    DEB_PL("failed to open tftFile");
  }
  return tftFileSize;
}

size_t Storage::readUpdateFile(size_t bytesToRead, uint8_t* buffer)
{
  return tftFile.readBytes((char*)buffer, bytesToRead);
}

void Storage::closeUpdateFile()
{
  tftFile.close();
  DEB_PL("tftFile closed");
  delay(100);
  LITTLEFS.remove(nextionTftFile);
  delay(2000);
  DEB_PL("tftFile removed");
}


int32_t Storage::getCurrentFuelPriceLimit(const FuelType fuelType, FuelStations& stationList)
{
  TRACE();

  int32_t retValue = 100;

  if (!prefs.begin(settingsNamespace, false))
  {
    DEB_PL("open preferences namespace 'settings' failed");
    return -1;
  }
  else
  {
    DEB_PL("namespace 'settings' is initialized");
    switch (fuelType)
    {
      case FuelType::DIESEL:
        retValue = prefs.getInt(settingsKeyLimitDiesel, -1.0);
        break;
      case FuelType::SUPER:
        retValue = prefs.getInt(settingsKeyLimitSuper, -1.0);
        break;
      case FuelType::SUPER_E10:
        retValue = prefs.getInt(settingsKeyLimitSuperE10, -1.0);
        break;
    }
    if (retValue < 0)
    {
      int32_t retError = 0;

      retValue = 0.0;
      DEB_P("write initial price limit ");
      switch (fuelType)
      {
        case FuelType::DIESEL:
          DEB_P("Diesel: ");
          retError = prefs.putInt(settingsKeyLimitDiesel, initialLimitDiesel);
          break;
        case FuelType::SUPER:
          retError = prefs.putInt(settingsKeyLimitSuper, initialLimitSuper);
          DEB_P("Super : ");
          break;
        case FuelType::SUPER_E10:
          retError = prefs.putInt(settingsKeyLimitSuperE10, initialLimitSuperE10);
          DEB_P("Super : ");
          break;
      }
      DEB_PF("%4.2f\n", (float)retValue/100.);

      if (retError == 0)
      {
        DEB_PF("write initial limit value failed\n");
        prefs.end();
        return -2;
      }
    }
    else
    {
      DEB_PF("current %s ", fuelTypeName(fuelType));
      DEB_PF("limit: %4.2f\n", (float)retValue/100.);
    }
    prefs.end();
  }

  // save value to fuel stations object
  stationList.setLimit(fuelType, retValue);
  
  // save value internally to avoid multiple writes of same value
  saveLastFuelPriceLimit(fuelType, retValue);

  return retValue;
}

bool Storage::putCurrentFuelPriceLimit(const FuelType fuelType, const int32_t value)
{
  TRACE();
  int32_t lastValue = 100;


  if (!prefs.begin(settingsNamespace, false))
  {
    DEB_PL("open preferences namespace 'settings' failed");
    return false;
  }
  else
  {
    switch (fuelType)
    {
      case FuelType::DIESEL:
        lastValue = lastDieselLimit;
        break;
      case FuelType::SUPER:
        lastValue = lastSuperLimit;
        break;
      case FuelType::SUPER_E10:
        lastValue = lastSuperE10Limit;
        break;
    }
    if (value == lastValue)
    {
      // nothing to do: value is already there
      prefs.end();
      return true;
    }
    DEB_PL("namespace 'settings' is initialized");
    size_t retValue = 0;
    switch (fuelType)
    {
      case FuelType::DIESEL:
        retValue = prefs.putInt(settingsKeyLimitDiesel, value);
        break;
      case FuelType::SUPER:
        retValue = prefs.putInt(settingsKeyLimitSuper, value);
        break;
      case FuelType::SUPER_E10:
        retValue = prefs.putInt(settingsKeyLimitSuperE10, value);
        break;
    }
    if (retValue == 0)
    {
      DEB_PF("write current fuel price limit %4.2f failed\n", (float)value/100.);
      return false;
    }
    DEB_PF("current fuel price limit %4.2f written\n", (float)value/100.);
    prefs.end();
  }
  // save value internally to avoid multiple writes of same value
  saveLastFuelPriceLimit(fuelType, value);

  return true;
}

void Storage::saveLastFuelPriceLimit(const FuelType fuelType, const int32_t value)
{
  switch (fuelType)
  {
    case FuelType::DIESEL:
      lastDieselLimit = value;
      break;
    case FuelType::SUPER:
      lastSuperLimit = value;
      break;
    case FuelType::SUPER_E10:
      lastSuperE10Limit = value;
      break;
  }
}

bool Storage::getStationList(FuelStations& stationList)
{
  TRACE();

  // station list file
  File stations = LITTLEFS.open(fuelDataFile);
  if (stations && !stations.isDirectory())
  {
    DEB_PL("fuel stations list file open");
    size_t  siz = stations.size();
    char* buf = new char[siz + 1];
    if (buf)
    {
      stations.readBytes(buf, siz);
      stations.close();
      buf[siz] = 0;
      DEB_PF("  %zu bytes read\n", siz);
      DEB_PL("fuel stations list file closed");

      StaticJsonDocument<jsonFuelStationListDocSize> doc;
      DeserializationError error = deserializeJson(doc, buf, siz);
      if (error)
      {
        DEB_P("deserializeJson() failed: ");
        DEB_PL(error.c_str());
        return false;
      }

      stationList.setAPIKey(doc["APIkey"]);
      JsonArray stationListArray = doc["StationList"].as<JsonArray>();
      stationList.createStationList(stationListArray.size());

      int count = 0;
      FuelStation current;
      for (JsonObject StationList_item : stationListArray)
      {

        current.setUiName(StationList_item["uiname"]);
        current.setSpeechName(StationList_item["spname"]);
        current.setSpeechCity(StationList_item["spcity"]);
        current.setId(StationList_item["key"]);
        stationList.setStation(count, current);
        count++;
      }
    }
    else
    {
      stations.close();
      DEB_PF("storing %zu bytes from file failed\n", siz);
    }
  }
  else
  {
    DEB_PL("fule station list file not found");
  }

  return true;
}
