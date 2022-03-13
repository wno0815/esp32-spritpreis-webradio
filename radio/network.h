#pragma once
#include <Arduino.h>
#include <limits.h>
#include <WiFi.h>

#include "trace.h"

/*
   Holds predefined network credentials from configuration file
*/
class Network
{
  public:
    Network();

    // This constructor is not used so far.
    Network(const char* networkSSID, const char* networkPassword);

    /*
       set Network attributes
       used when reading from storage or getting from Web for initial setting
    */
    void setSSID(const char* networkSSID);
    void setPassword(const char* networkPassword);
    void setScanIndex(int32_t networkScanIndex);

    /*
       get Network attributes
    */
    const char* getSSID();
    const char* getPassword();
    int32_t getScanIndex();

    /*
       print this object to debug
    */
    void debugPrint(bool suppressPassword = true);

  private:
    const char* ssid;                 // network SSID
    const char* password;             // network password (key phrase)
    int32_t     scanIndex {INT_MAX};  // index in list of available networks
};

// ============================================================================================================================

class Networks
{
  public:
    Networks();

    /*
       Networks defined in list (network.json) in file system
    */
    int32_t getNumberOfNetworks();
    bool createNetworkList(int32_t number);
    bool setNetwork(int32_t index, Network & network);
    Network & getNetwork(int32_t index);
    Network& operator[](int32_t index);

    /*
       networks on air
    */
    int32_t getNumberOfAvailableNetworks();
    int32_t createAvailableNetworkList();
    int32_t matchNetwork();

    /*
       handle connection
    */
    bool connectNetwork();
    bool disconnectNetwork();
    bool checkNetwork();
    const char* getCurrentName();
    const char* getCurrentIP();

    /*
       available Networks in current environment
    */

    void debugPrint(bool suppressPassword = true);

  private:
    int32_t numberOfNetworks { -1};
    Network* networkList;
    int32_t numberOfAvailableNetworks { -1};
    int32_t indexOfSelectedNetwork {INT_MAX};

    // handle connection
    bool isConnected = false;
    unsigned long lastCheckTime;
    const unsigned long checkInterval {5000};
};
