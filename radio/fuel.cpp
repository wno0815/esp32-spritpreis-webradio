
#include "fuel.h"

//#define TANKERKOENIG_VERBOSE

#include <HTTPClient.h>
#define ARDUINOJSON_USE_LONG_LONG 0
#define ARDUINOJSON_USE_DOUBLE 0
#include <ArduinoJson.h>


// RootCA von Tankerkoenig
const char* root_ca = \
                      "-----BEGIN CERTIFICATE-----\n" \
                      "MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw\n" \
                      "TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n" \
                      "cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4\n" \
                      "WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu\n" \
                      "ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY\n" \
                      "MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc\n" \
                      "h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+\n" \
                      "0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U\n" \
                      "A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW\n" \
                      "T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH\n" \
                      "B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC\n" \
                      "B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv\n" \
                      "KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn\n" \
                      "OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn\n" \
                      "jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw\n" \
                      "qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI\n" \
                      "rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV\n" \
                      "HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq\n" \
                      "hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL\n" \
                      "ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ\n" \
                      "3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK\n" \
                      "NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5\n" \
                      "ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur\n" \
                      "TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC\n" \
                      "jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc\n" \
                      "oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq\n" \
                      "4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA\n" \
                      "mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d\n" \
                      "emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=\n" \
                      "-----END CERTIFICATE-----\n";

String fuelJsonBuffer;


const char* fuelTypeName(FuelType fuelType)
{
  switch (fuelType)
  {
    case FuelType::DIESEL:
      return "DIESEL";
      break;
    case FuelType::SUPER:
      return "SUPER";
      break;
    case FuelType::SUPER_E10:
      return "SUPER E10";
      break;
  }
  return "unknown";
}

// ============================================================================================================================

FuelStation::FuelStation() {}

void FuelStation::setUiName(const char* stationName)
{
  uiName = stationName;
}

void FuelStation::setSpeechName(const char* stationName)
{
  speechName = stationName;
}

void FuelStation::setSpeechCity(const char* stationCity)
{
  speechCity = stationCity;
}

void FuelStation::setId(const char* stationId)
{
  id = stationId;
}

void FuelStation::setIsOpen(const bool value)
{
  if (!value)
  {
    // reset all alarms
    activateAlarm(FuelType::DIESEL, false);
    activateAlarm(FuelType::SUPER, false);
    activateAlarm(FuelType::SUPER_E10, false);
  }
  isOpenFlag = value;
}

void FuelStation::setPrice(const FuelType fuelType, const float value)
{
  switch (fuelType)
  {
    case FuelType::DIESEL:
      priceDiesel = value;
      break;
    case FuelType::SUPER:
      priceE5 = value;
      break;
    case FuelType::SUPER_E10:
      priceE10 = value;
      break;
  }
}

const char* FuelStation::getUiName()
{
  return uiName;
}

const char* FuelStation::getSpeechName()
{
  return speechName;
}

const char* FuelStation::getSpeechCity()
{
  return speechCity;
}

const char* FuelStation::getId()
{
  return id;
}

bool FuelStation::isOpen()
{
  return isOpenFlag;
}

float FuelStation::getPrice(const FuelType fuelType)
{
  if (isOpen())
  {
    switch (fuelType)
    {
      case FuelType::DIESEL:
        return priceDiesel;
        break;
      case FuelType::SUPER:
        return priceE5;
        break;
      case FuelType::SUPER_E10:
        return priceE10;
        break;
    }
  }
  return 0.0;
}

void FuelStation::activateAlarm(const FuelType fuelType, const bool activate)
{
  if (isOpen())
  {
    switch (fuelType)
    {
      case FuelType::DIESEL:
        alarmDiesel = activate;
        break;
      case FuelType::SUPER:
        alarmE5 = activate;
        break;
      case FuelType::SUPER_E10:
        alarmE10 = activate;
        break;
    }
  }
}

bool FuelStation::getAlarm(const FuelType fuelType)
{
  if (isOpen())
  {
    switch (fuelType)
    {
      case FuelType::DIESEL:
        return alarmDiesel;
        break;
      case FuelType::SUPER:
        return alarmE5;
        break;
      case FuelType::SUPER_E10:
        return alarmE10;
        break;
    }
  }
  return false;
}

const char* getFuelTypeName(const FuelType fuelType)
{
  switch (fuelType)
  {
    case FuelType::DIESEL:
      return "Diesel";
      break;
    case FuelType::SUPER:
      return "Super";
      break;
    case FuelType::SUPER_E10:
      return "Super E 10";
      break;
  }
  return "";
}

char* FuelStation::createAlarmText(const FuelType fuelType)
{
  TRACE();
  memset(alarmText, 0, alarmTextLength);
  if (getAlarm(fuelType))
  {
    DEB_PF("alarm active for %s\n", getFuelTypeName(fuelType));
    int32_t price = getPrice(fuelType) * 1000.;
    int32_t euros = price / 1000;
    int32_t cents = (price - euros * 1000) / 10;
    int32_t rest  = price - euros * 1000 - cents * 10;
    DEB_PF("%s bei %s in %s jetzt %d Euro %d %d\n",
           getFuelTypeName(fuelType), speechName, speechCity, euros, cents, rest);
    snprintf(alarmText, alarmTextLength, "%s bei %s in %s jetzt %d Euro %d %d. ",
             getFuelTypeName(fuelType), speechName, speechCity, euros, cents, rest);
  }
  return alarmText;
}

void FuelStation::debugPrint()
{
  if (isOpenFlag)
  {
    DEB_PF(" %s  Diesel: %5.3f  E5: %5.3f  E10: %5.3f\n", uiName, priceDiesel, priceE5, priceE10);
  }
  else
  {
    DEB_PF(" %s  closed\n", uiName);
  }
}

// ============================================================================================================================

FuelStations::FuelStations() {}

uint32_t FuelStations::getNumberOfStations()
{
  return numberOfStations;
}

void FuelStations::setAPIKey(const char* keyString)
{
  APIKey = keyString;
}

bool FuelStations::createStationList(uint32_t number)
{
  TRACE();

  if (stationList != NULL)
  {
    DEB_PF("station list is not empty ( %d elements). Deleting.\n", numberOfStations);
    delete stationList;
  }
  numberOfStations = number;
  stationList = new FuelStation[numberOfStations]();
  if (stationList == NULL)
  {
    DEB_PL("creation of station list failed");
    return false;
  }
  DEB_PF("station list created for %d elements\n", numberOfStations);
  return true;
}

bool FuelStations::setStation(uint32_t index, FuelStation& station)
{
  if (index >= numberOfStations)
  {
    DEB_PF("FuelStations::setStation : index %d is too large for list of %d elements\n", index, numberOfStations);
    return false;
  }
  stationList[index] = station;
  DEB_PF("station[ %d] set to '%s'\n", index, stationList[index].getUiName());
  return true;
}

FuelStation& FuelStations::getStation(uint32_t index)
{
  if (index >= numberOfStations)
  {
    DEB_PF("FUEL : index %d is too large for list of %d elements\n", index, numberOfStations);
    return stationList[0];
  }
  return stationList[index];
}

FuelStation& FuelStations::operator[](uint32_t index)
{
  return getStation(index);
}


bool FuelStations::isBelowLimit(const FuelType fuelType, const int32_t stationIndex)
{
  if (stationList[stationIndex].isOpen())
  {
    switch (fuelType)
    {
      case FuelType::DIESEL:
        return (stationList[stationIndex].getPrice(fuelType) < floatLimitDiesel);
        break;
      case FuelType::SUPER:
        return (stationList[stationIndex].getPrice(fuelType) < floatLimitE5);
        break;
      case FuelType::SUPER_E10:
        return (stationList[stationIndex].getPrice(fuelType) < floatLimitE10);
        break;
    }
  }
  return false;
}

void FuelStations::setLimit(const FuelType fuelType, const int32_t value)
{
  switch (fuelType)
  {
    case FuelType::DIESEL:
      limitDiesel = value;
      floatLimitDiesel = (float)value / 100.;
      break;
    case FuelType::SUPER:
      limitE5 = value;
      floatLimitE5 = (float)value / 100.;
      break;
    case FuelType::SUPER_E10:
      limitE10 = value;
      floatLimitE10 = (float)value / 100.;
      break;
  }
  DEB_PF("FUEL : limit for %s set to %4.2f\n", fuelTypeName(fuelType), (float)value / 100.);
}

int32_t FuelStations::getLimit(const FuelType fuelType)
{
  switch (fuelType)
  {
    case FuelType::DIESEL:
      return limitDiesel;
      break;
    case FuelType::SUPER:
      return limitE5;
      break;
    case FuelType::SUPER_E10:
      return limitE10;
      break;
  }
  return -1;
}

int32_t FuelStations::getCurrentStationIndex()
{
  return currentStationIndex;
}

bool FuelStations::selectNextPrevious(bool next)
{
  TRACE();
  if (next)
  {
    if (currentStationIndex < numberOfStations - 1)
    {
      currentStationIndex++;
    }
    else
    {
      currentStationIndex = 0;
    }
  }
  else
  {
    if (currentStationIndex == 0)
    {
      currentStationIndex = numberOfStations - 1;
    }
    else
    {
      currentStationIndex--;
    }
  }

  return true;
}

bool FuelStations::checkLimits()
{
  bool retValue = false;
  int32_t firstAlarmIndex = -1;

  DEB_PL("FUEL : check limits");
  for (int32_t count = 0; count < numberOfStations; count++)
  {
    if (isBelowLimit(FuelType::DIESEL, count))
    {
      stationList[count].activateAlarm(FuelType::DIESEL, true);
      retValue = true;
      if (firstAlarmIndex == -1)
        firstAlarmIndex = count;
      DEB_PF("    Diesel %5.3f below limit %5.3f at %s\n", stationList[count].getPrice(FuelType::DIESEL), floatLimitDiesel, stationList[count].getUiName());
    }
    else
    {
      stationList[count].activateAlarm(FuelType::DIESEL, false);
    }

    if (isBelowLimit(FuelType::SUPER, count))
    {
      stationList[count].activateAlarm(FuelType::SUPER, true);
      retValue = true;
      if (firstAlarmIndex == -1)
        firstAlarmIndex = count;
      DEB_PF("    Super  %5.3f below limit %5.3f at %s\n", stationList[count].getPrice(FuelType::SUPER), floatLimitE5, stationList[count].getUiName());
    }
    else
    {
      stationList[count].activateAlarm(FuelType::SUPER, false);
    }

    if (isBelowLimit(FuelType::SUPER_E10, count))
    {
      stationList[count].activateAlarm(FuelType::SUPER_E10, true);
      retValue = true;
      if (firstAlarmIndex == -1)
        firstAlarmIndex = count;
      DEB_PF("    Super E10  %5.3f below limit %5.3f at %s\n", stationList[count].getPrice(FuelType::SUPER_E10), floatLimitE10, stationList[count].getUiName());
    }
    else
    {
      stationList[count].activateAlarm(FuelType::SUPER_E10, false);
    }
  }

  if (retValue)
  {
    currentStationIndex = firstAlarmIndex;
  }

  return retValue;
}

char* FuelStations::getAlarmText(const FuelType fuelType)
{
  TRACE();
  memset(alarmText, 0, alarmTextLength);
  strncat(alarmText, stationList[currentStationIndex].createAlarmText(fuelType), alarmTextLength);
  DEB_PF("    alarm text : '%s'\n", alarmText);
  return alarmText;
}

void FuelStations::debugPrint()
{
  DEB_PL("Fuel station list : ");
  DEB_PF("    limit price Diesel   : %4.2f€\n", floatLimitDiesel);
  DEB_PF("    limit price Super    : %4.2f€\n", floatLimitE5);
  DEB_PF("    limit price Super E10 : %4.2f€\n", floatLimitE10);
  DEB_PF("    number of stations : %d\n", numberOfStations);
  DEB_PL("    station list : ");
  for (uint32_t count = 0; count < numberOfStations; count++)
  {
    DEB_PF("       [ %3d]  ", count);
    stationList[count].debugPrint();
  }
}

String FuelStations::httpGETRequest(const char* serverName)
{
  if (UseTankerkoenigFakeValues)
  {
    // Test: return fake results
    static int callNumber = 0;
    callNumber++;
    if (callNumber > 5)
      callNumber = 1;
    DEB_PF("    [ %2d] fake result\n", callNumber);
    // assume: Limit Diesel = 1.40, Super = 1.50  (Super E10 not used)
    switch (callNumber)
    {
      case 1:
        // all stations open, all prices above limit
        return String(" {\"ok\":true,\"license\":\"CC BY 4.0 -  https://creativecommons.tankerkoenig.de\",\"data\":\"MTS-K\",\"prices\":{\"1a1ec4ba-cc2a-4663-8330-81efc48b9256\":{\"status\":\"open\",\"e5\":1.719,\"e10\":1.699,\"diesel\":1.579},\"3c034790-3d2a-4093-8417-032a48cb9f25\":{\"status\":\"open\",\"e5\":1.699,\"e10\":1.649,\"diesel\":1.589},\"e1a15081-24c2-9107-e040-0b0a3dfe563c\":{\"status\":\"open\",\"e5\":1.669,\"e10\":1.659,\"diesel\":1.549},\"51d4b5e1-a095-1aa0-e100-80009459e03a\":{\"status\":\"open\",\"e5\":1.689,\"e10\":1.679,\"diesel\":1.569}}}");
        break;
      case 2:
        // Holle closed, all prices above limit
        return String("{\"ok\":true,\"license\":\"CC BY 4.0 -  https://creativecommons.tankerkoenig.de\",\"data\":\"MTS-K\",\"prices\":{\"1a1ec4ba-cc2a-4663-8330-81efc48b9256\":{\"status\":\"open\",\"e5\":1.719,\"e10\":1.699,\"diesel\":1.579},\"3c034790-3d2a-4093-8417-032a48cb9f25\":{\"status\":\"closed\"},\"e1a15081-24c2-9107-e040-0b0a3dfe563c\":{\"status\":\"open\",\"e5\":1.669,\"e10\":1.659,\"diesel\":1.549},\"51d4b5e1-a095-1aa0-e100-80009459e03a\":{\"status\":\"open\",\"e5\":1.689,\"e10\":1.679,\"diesel\":1.569}}}");
        break;
      case 3:
        // all stations open, Gross Duengen Dieesl below, Ochtersum both below
        return String("{\"ok\":true,\"license\":\"CC BY 4.0 -  https://creativecommons.tankerkoenig.de\",\"data\":\"MTS-K\",\"prices\":{\"1a1ec4ba-cc2a-4663-8330-81efc48b9256\":{\"status\":\"open\",\"e5\":1.719,\"e10\":1.699,\"diesel\":1.579},\"3c034790-3d2a-4093-8417-032a48cb9f25\":{\"status\":\"open\",\"e5\":1.699,\"e10\":1.649,\"diesel\":1.529},\"e1a15081-24c2-9107-e040-0b0a3dfe563c\":{\"status\":\"open\",\"e5\":1.619,\"e10\":1.659,\"diesel\":1.449},\"51d4b5e1-a095-1aa0-e100-80009459e03a\":{\"status\":\"open\",\"e5\":1.559,\"e10\":1.629,\"diesel\":1.379}}}");
        break;
      case 4:
        // all stations open, all prices above limit
        return String(" {\"ok\":true,\"license\":\"CC BY 4.0 -  https://creativecommons.tankerkoenig.de\",\"data\":\"MTS-K\",\"prices\":{\"1a1ec4ba-cc2a-4663-8330-81efc48b9256\":{\"status\":\"open\",\"e5\":1.719,\"e10\":1.699,\"diesel\":1.579},\"3c034790-3d2a-4093-8417-032a48cb9f25\":{\"status\":\"open\",\"e5\":1.699,\"e10\":1.649,\"diesel\":1.589},\"e1a15081-24c2-9107-e040-0b0a3dfe563c\":{\"status\":\"open\",\"e5\":1.669,\"e10\":1.659,\"diesel\":1.549},\"51d4b5e1-a095-1aa0-e100-80009459e03a\":{\"status\":\"open\",\"e5\":1.689,\"e10\":1.679,\"diesel\":1.569}}}");
        break;
      case 5:
        // all stations open, Holle Diesel & Super below limit
        return String(" {\"ok\":true,\"license\":\"CC BY 4.0 -  https://creativecommons.tankerkoenig.de\",\"data\":\"MTS-K\",\"prices\":{\"1a1ec4ba-cc2a-4663-8330-81efc48b9256\":{\"status\":\"open\",\"e5\":1.719,\"e10\":1.699,\"diesel\":1.579},\"3c034790-3d2a-4093-8417-032a48cb9f25\":{\"status\":\"open\",\"e5\":1.589,\"e10\":1.649,\"diesel\":1.479},\"e1a15081-24c2-9107-e040-0b0a3dfe563c\":{\"status\":\"open\",\"e5\":1.669,\"e10\":1.659,\"diesel\":1.549},\"51d4b5e1-a095-1aa0-e100-80009459e03a\":{\"status\":\"open\",\"e5\":1.689,\"e10\":1.679,\"diesel\":1.569}}}");
        break;
    }
    return String("{\"ok\":true,\"license\":\"CC BY 4.0 -  https://creativecommons.tankerkoenig.de\",\"data\":\"MTS-K\",\"prices\":{\"1a1ec4ba-cc2a-4663-8330-81efc48b9256\":{\"status\":\"closed\"},\"3c034790-3d2a-4093-8417-032a48cb9f25\":{\"status\":\"closed\"},\"e1a15081-24c2-9107-e040-0b0a3dfe563c\":{\"status\":\"closed\"},\"51d4b5e1-a095-1aa0-e100-80009459e03a\":{\"status\":\"closed\"}}}");
  }

  WiFiClient client;
  HTTPClient http;

  // Your Domain name with URL path or IP address with path
  http.begin(serverName, root_ca);

  // Send HTTP POST request
  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0)
  {
#if defined(TANKERKOENIG_VERBOSE)
    DEB_P("HTTP Response code: ");
    DEB_PL(httpResponseCode);
#endif
    payload = http.getString();
  }
  else
  {
    DEB_P("Error code: ");
    DEB_PL(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

bool FuelStations::updatePrices(const bool closeAllStations)
{

  DEB_PL("TANKERKOENIG: Update");

  // used when outside fuel scan time window
  if (closeAllStations)
  {
    for (uint32_t count = 0; count < numberOfStations; count++)
    {
      stationList[count].setIsOpen(false);
    }
    return true;
  }

  String serverPath = "https://creativecommons.tankerkoenig.de/json/prices.php?ids=";
  for (uint32_t count = 0; count < numberOfStations; count++)
  {
    serverPath += stationList[count].getId();
    serverPath += ',';
  }
  int lastKomma = serverPath.lastIndexOf(',');
  serverPath[lastKomma] = '&';   // ersetze das letzte Komma durch ein & (was vorher am Anfang des apikey-Strings stand)
  serverPath += "apikey=" + String(APIKey);

#if defined(TANKERKOENIG_VERBOSE)
  DEB_P("TANKERKOENIG: RequestString\n    '");
  DEB_P(serverPath);
  DEB_PL("'");
#endif
  // TEST: don't call server until request string is correct
  //return true;

  fuelJsonBuffer = httpGETRequest(serverPath.c_str());
#if defined(TANKERKOENIG_VERBOSE)
  DEB_P("    ");
  DEB_PL(fuelJsonBuffer);
#endif

  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, fuelJsonBuffer);

  if (error)
  {
    DEB_P("deserializeJson() failed: ");
    DEB_PL(error.f_str());
    return false;
  }

  // von Tommy
  bool isOk = doc["ok"];
  if (!isOk)
  {
    String msg = doc["message"];
    DEB_PL(msg);
    return false;
  }

  JsonObject prices = doc["prices"];
  for (uint32_t count = 0; count < numberOfStations; count++)
  {
    DEB_PL(stationList[count].getUiName());
    JsonObject station = prices[stationList[count].getId()];
    if (station["status"] == "closed")
    {
      stationList[count].setIsOpen(false);
      stationList[count].activateAlarm(FuelType::DIESEL, false);
      stationList[count].activateAlarm(FuelType::SUPER, false);
      stationList[count].activateAlarm(FuelType::SUPER_E10, false);
      DEB_PL("    geschlossen");
      continue;
    }
    if (station["status"] == "no prices")
    {
      DEB_PL("    keine Preise verfügbar");
      continue;
    }

    stationList[count].setIsOpen(true);
    float stationDiesel = station["diesel"];
    float stationE5 = station["e5"];
    float stationE10 = station["e10"];
    stationList[count].setPrice(FuelType::DIESEL, stationDiesel);
    stationList[count].setPrice(FuelType::SUPER,  stationE5);
    stationList[count].setPrice(FuelType::SUPER_E10,  stationE10);
    DEB_PF("    Diesel    : %5.3f\n", stationDiesel);
    DEB_PF("    Super E5  : %5.3f\n", stationE5);
    DEB_PF("    Super E10 : %5.3f\n", stationE10);
  }

  return true;
}
