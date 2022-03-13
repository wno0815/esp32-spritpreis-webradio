
#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "FS.h"
#include <LITTLEFS.h>

#include "trace.h"

#include "config.h"
#include "station.h"
#include "network.h"
#include "fuel.h"


class Storage
{
  public:
    bool begin();

    // web radio station related
    bool getStationList(RadioStations& stationList, RadioStationKeys& keyList);
    int32_t getCurrentStationIndex(RadioStations& stationList);
    bool putCurrentStationIndex(int32_t index);
    int32_t getCurrentBrightness();
    bool putCurrentBrightness(int32_t value);

    // network related
    bool getNetworkList(Networks& networkList);

    // Tankerkoenig 
    bool getStationList(FuelStations& stationList);
    int32_t getCurrentFuelPriceLimit(const FuelType fuelType, FuelStations& stationList);
    bool putCurrentFuelPriceLimit(const FuelType fuelType, const int32_t value);
    

// Nextion upload 
    size_t openUpdateFile();
    size_t readUpdateFile(size_t bytesToRead, uint8_t* buffer);
    void closeUpdateFile();

  private:
    Preferences prefs;
    int32_t lastCurrentStationIndex;
    int32_t lastBrightness;
    int32_t lastDieselLimit;
    int32_t lastSuperLimit;
    int32_t lastSuperE10Limit;
    void saveLastFuelPriceLimit(const FuelType fuelType, const int32_t value);

    
    File tftFile;
    size_t  tftFileSize = 0;

};
