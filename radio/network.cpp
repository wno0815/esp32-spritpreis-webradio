
#include "network.h"

#include <esp_wifi.h>
uint8_t newMACAddress[] = {0xac, 0x57, 0x75, 0x8d, 0xf8, 0xfb};

// ============================================================================================================================

Network::Network() {}
/*
   full attribute constructor
   might be used for hard coded networks
      Network homeNetwork("my home network SSID", "my home network passphraseF");
*/
Network::Network(const char* networkSSID, const char* networkPassword) :
  ssid {networkSSID}, password {networkPassword}, scanIndex { -1} {}

/*
   set Network attributes
*/
void Network::setSSID(const char* networkSSID)
{
  ssid = networkSSID;
}

void Network::setPassword(const char* networkPassword)
{
  password = networkPassword;
}

void Network::setScanIndex(int32_t networkScanIndex)
{
  scanIndex = networkScanIndex;
}

/*
   get Network attributes
*/
const char* Network::getSSID()
{
  return ssid;
}

const char* Network::getPassword()
{
  return password;
}

int32_t Network::getScanIndex()
{
  return scanIndex;
}

void Network::debugPrint(bool suppressPassword)
{
  if (suppressPassword)
  {
    DEB_PF("%s | %d\n", ssid, scanIndex);
  }
  else
  {
    DEB_PF("%s | %s | %d\n", ssid, password, scanIndex);
  }
}

// ============================================================================================================================

Networks::Networks() {};


int32_t Networks::getNumberOfNetworks()
{
  return numberOfNetworks;
}

bool Networks::createNetworkList(int32_t number)
{
  TRACE();

  if (networkList != NULL)
  {
    DEB_PF("network list is not empty (%d elements). Deleting.\n", numberOfNetworks);
    delete networkList;
  }
  numberOfNetworks = number;
  networkList = new Network[numberOfNetworks]();
  if (networkList == NULL)
  {
    DEB_PL("creation of network list failed");
    return false;
  }
  DEB_PF("network list created for %d elements\n", numberOfNetworks);
  return true;
}

bool Networks::setNetwork(int32_t index, Network & network)
{
  if (index >= numberOfNetworks)
  {
    DEB_PF("index %d is too large for list of %d elements\n", index, numberOfNetworks);
    return false;
  }
  networkList[index] = network;
  DEB_PF("network[%d] SSID '%s'\n", index, networkList[index].getSSID());
  return true;
}

Network& Networks::getNetwork(int32_t index)
{
  if (index >= numberOfNetworks)
  {
    DEB_PF("index %d is too large for list of %d elements\n", index, numberOfNetworks);
    return networkList[0];
  }
  return networkList[index];
}


Network& Networks::operator[](int32_t index)
{
  return getNetwork(index);
}

int32_t Networks::getNumberOfAvailableNetworks()
{
  return numberOfAvailableNetworks;
}

int32_t Networks::createAvailableNetworkList()
{
  TRACE();

  // creates network list internally; ordered by signal strength
  numberOfAvailableNetworks = WiFi.scanNetworks();

  DEB_P("Number of networks found: ");
  DEB_PL(numberOfAvailableNetworks);

  for (int i = 0; i < numberOfAvailableNetworks; i++)
  {

    DEB_PF("----- %2d -----\n", i);
    DEB_P("Network name   : ");
    DEB_PL(WiFi.SSID(i));

    DEB_P("Signal strength: ");
    DEB_PL(WiFi.RSSI(i));

    DEB_P("MAC address    : ");
    DEB_PL(WiFi.BSSIDstr(i));
  }
  return numberOfAvailableNetworks;
}

int32_t Networks::matchNetwork()
{
  TRACE();

  if (numberOfNetworks == -1 || numberOfAvailableNetworks == -1)
  {
    // at least one of the lists is empty - no match possible
    DEB_PL("no match possible");
    return indexOfSelectedNetwork;
  }

  for (int32_t networkCount = 0; networkCount < numberOfNetworks; networkCount++)
  {
    DEB_PF("search '%s' in ", networkList[networkCount].getSSID());
    for (int32_t availableNetworkCount = 0; availableNetworkCount < numberOfAvailableNetworks; availableNetworkCount++)
    {
      DEB_PF("'%s' ", WiFi.SSID(availableNetworkCount).c_str());
      if (strcmp(networkList[networkCount].getSSID(), WiFi.SSID(availableNetworkCount).c_str()) == 0)
      {
        networkList[networkCount].setScanIndex(availableNetworkCount);
        // network found; leave inner loop here and try next network
        DEB_P("found");
        break;
      }
    }
    DEB_PL();
  }

  // all networks in configuration now have a scanIndex value.
  // Lowest index is highest signal strength
  DEB_P("search best match network...");
  int32_t lowestScanIndex = INT_MAX;
  for (int32_t networkCount = 0; networkCount < numberOfNetworks; networkCount++)
  {
    if (networkList[networkCount].getScanIndex() < lowestScanIndex)
    {
      lowestScanIndex = networkList[networkCount].getScanIndex();
      indexOfSelectedNetwork = networkCount;
    }
  }

  // network selected
  if (indexOfSelectedNetwork == INT_MAX)
  {
    // index still holds initialisation value = not found
    DEB_PL("not found");
  }
  else
  {
    DEB_PF(" found at index %d: '%s' [%d]\n", indexOfSelectedNetwork, networkList[indexOfSelectedNetwork].getSSID(), networkList[indexOfSelectedNetwork].getScanIndex());
  }

  return indexOfSelectedNetwork;
}

bool Networks::connectNetwork()
{
  TRACE();
  if (indexOfSelectedNetwork == INT_MAX)
  {
    DEB_PL("no matched network available for connection");
    return false;
  }

  DEB_PF("connect to '%s'", networkList[indexOfSelectedNetwork].getSSID());
  WiFi.mode(WIFI_STA);
  WiFi.begin(networkList[indexOfSelectedNetwork].getSSID(), networkList[indexOfSelectedNetwork].getPassword());
  int count = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    DEB_P('.');
    count++;
    if (count > 20)
      break;
  }

  if (count > 20)
  {
    DEB_PL(" failed");
    return false;
  }
  DEB_PL(" successful");
  DEB_P("IP address is ");
  DEB_PL(WiFi.localIP());

  isConnected = true;
  return true;
}

bool Networks::disconnectNetwork()
{
  TRACE();

  if (isConnected)
  {
    WiFi.disconnect();
    isConnected = false;
  }
  return isConnected;
}

bool Networks::checkNetwork()
{
  unsigned long currentTime = millis();

  // if WiFi is down, try reconnecting
  if ((WiFi.status() != WL_CONNECTED) && (currentTime - lastCheckTime >= checkInterval))
  {
    TRACE();
    DEB_PL("network connection lost; try reconnect");
    disconnectNetwork();
    connectNetwork();
    lastCheckTime = currentTime;
  }
  return isConnected;
}

const char* Networks::getCurrentName()
{
  if (isConnected)
  {
    return networkList[indexOfSelectedNetwork].getSSID();
  }
  return "";
}

const char* Networks::getCurrentIP()
{
  if (isConnected)
  {
    return WiFi.localIP().toString().c_str();
  }
  return "";
}


void Networks::debugPrint(bool suppressPassword)
{
  DEB_PL("Network list:");
  DEB_PF("    number of networks: %d\n", numberOfNetworks);
  DEB_PL("    network list:");
  for (int32_t count = 0; count < numberOfNetworks; count++)
  {
    DEB_PF("       [%2d]  ", count);
    networkList[count].debugPrint(suppressPassword);
  }
  DEB_PF("    number of available networks: %d\n", numberOfAvailableNetworks);
  for (int32_t count = 0; count < numberOfAvailableNetworks; count++)
  {
    DEB_PF("       [%2d] %s | %d | %s\n", count, WiFi.SSID(count).c_str(), WiFi.RSSI(count), WiFi.BSSIDstr(count).c_str());
  }
  DEB_PF("    network '%s' is %sconnected\n", indexOfSelectedNetwork == INT_MAX ? "none" : networkList[indexOfSelectedNetwork].getSSID(), isConnected ? "" : "not " );
}
