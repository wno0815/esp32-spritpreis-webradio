/*
   Tankerekönig API

   Lizenz          : https://creativecommons.org/licenses/by/4.0/deed.de
   API Beschreibung: https://creativecommons.tankerkoenig.de/
   Hauptseite      : https://www.tankerkoenig.de/
*/

#pragma once
#include <Arduino.h>

#include "trace.h"
#include "config.h"

enum class FuelType { DIESEL, SUPER, SUPER_E10 };
const char* fuelTypeName(FuelType fuelType);


class FuelStation
{
  public:
    FuelStation();

    void setUiName(const char* stationName);
    void setSpeechName(const char* stationName);
    void setSpeechCity(const char* stationCity);
    void setId(const char* stationId);
    void setIsOpen(const bool value);
    void setPrice(const FuelType fuelType, const float value);

    const char* getUiName();
    const char* getSpeechName();
    const char* getSpeechCity();
    const char* getId();
    bool isOpen();
    float getPrice(const FuelType fuelType);
    void activateAlarm(const FuelType fuelType, const bool activate);
    bool getAlarm(const FuelType fuelType);
    char* createAlarmText(const FuelType fuelType);

    void debugPrint();

  private:
    const char* uiName;
    const char* speechName;
    const char* speechCity;
    const char* id;
    bool isOpenFlag = false;
    float priceE5;
    float priceE10;
    float priceDiesel;
    bool alarmE5 = false;
    bool alarmE10 = false;
    bool alarmDiesel = false;
    char alarmText[alarmTextLength];
};

// ============================================================================================================================
class FuelStations
{
    /*
       List of fuel stations found in file tanken.json
    */
  public:
    FuelStations();

    uint32_t getNumberOfStations();
    void setAPIKey(const char* keyString);
    bool createStationList(uint32_t number);
    bool setStation(uint32_t index, FuelStation& station);
    FuelStation& getStation(uint32_t index);
    FuelStation& operator[](uint32_t index);
    void setLimit(const FuelType fuelType, const int32_t value);
    int32_t getLimit(const FuelType fuelType);
    bool isBelowLimit(const FuelType fuelType, const int32_t stationIndex = 0);
    bool updatePrices(const bool closeAllStations = false);
    int32_t getCurrentStationIndex();
    bool selectNextPrevious(bool next);
    bool checkLimits();
    char* getAlarmText(const FuelType fuelType = FuelType::SUPER);

    void debugPrint();

  private:
    const char* APIKey;
    uint32_t numberOfStations;
    FuelStation* stationList;
    int32_t limitDiesel;
    int32_t limitE5;
    int32_t limitE10;
    float floatLimitDiesel;
    float floatLimitE5;
    float floatLimitE10;
    int32_t currentStationIndex = 0;
    String httpGETRequest(const char* serverName);
    char alarmText[alarmTextLength];
};
