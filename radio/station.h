
#pragma once
#include <Arduino.h>

#include "trace.h"
#include "config.h"


class RadioStation
{
    /*
       Holds information for a single WebRadio station.

       These are
          Name       to be used in UI (station names provided by broadcasters are sometimes ugly)
          KeyName    to be used in UI as label in station keys (limited space, keep short)
          Key        Number of station ("memory") key which shall select this station
                     Conditions:
                        Must not be larger than numberOfStationKey-1
                        Use 0 if not used (station in list only, but not on key)
          Url        URL used for playing the station (see broadcasters information)
    */
  public:
    RadioStation();
    /*
       const char* stationName;       // full name of station (intended for use by UI when played)
       const char* stationKeyName;    // short name for station keys (for UI station keys)
       uint32_t    stationKey;        // station key number (assignement to specific startion key)
       const char* stationUrl;          // stream URL

       This constructor is not used so far. Intended for use with hard coded stations
    */
    RadioStation(const char* stationName, const char* stationKeyName, uint32_t stationKey, const char* stationUrl);

    /*
       set RadioStation attributes
    */
    void setName(const char* stationName);
    void setKeyName(const char* stationKeyName);
    void setKey(uint32_t stationKey);
    void setUrl(const char* stationUrl);

    /*
       get RadioStation attributes
    */
    const char* getName();
    const char* getKeyName();
    uint32_t    getKey();
    const char* getUrl();

    /*
       print this object to debug
    */
    void debugPrint();

  private:
    const char* uiName;       // full name of station (used when played)
    const char* uiKeyName;    // short name for station keys (depends on UI)
    uint32_t    uiKey;        // station key number
    const char* url;          // stream URL

};

// ============================================================================================================================
class RadioStations
{
    /*
       List of all WebRadio stations found in file stations.json
    */
  public:
    RadioStations();

    bool setDefaultStationIndex(int32_t index);
    int32_t getDefaultStationIndex();
    uint32_t getNumberOfStations();

    bool createStationList(uint32_t number);
    bool setStation(uint32_t index, RadioStation& station);
    RadioStation& getStation(uint32_t index);
    RadioStation& operator[](uint32_t index);

    void debugPrint();

  private:
    int32_t  defaultStationIndex;
    uint32_t numberOfStations;
    RadioStation* stationList;
};

// ============================================================================================================================
class RadioStationKeys
{
  private:
    int32_t stationKeys[numberOfStationKeys];

  public:
    RadioStationKeys();

    bool setStationIndex(uint8_t key, int32_t index);
    int32_t getStationIndex(uint8_t key);

    void debugPrint();
    void debugPrint(RadioStations& stations);
};
