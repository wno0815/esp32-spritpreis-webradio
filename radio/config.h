
#pragma once

//=====================================================================================================
// configuration
// 
// files
constexpr char radioStationsFile[] = "/stations.json";
constexpr char networkCredentialsFile[] = "/network.json";
constexpr char fuelDataFile[] = "/tanken.json";
constexpr char nextionTftFile[] = "/radio.tft";
constexpr char gongFile[] = "/gong.mp3";
// nvs
constexpr char settingsNamespace[] = "settings";
constexpr char settingsKeyCurrentStation[] = "CurrStatIdx";
constexpr char settingsKeyBrightness[] = "CurrBright";
constexpr char settingsKeyLimitDiesel[] = "LimitDiesel";
constexpr char settingsKeyLimitSuper[] = "LimitSuper";
constexpr char settingsKeyLimitSuperE10[] = "LimitE10";

// json decoding
constexpr uint32_t jsonRadioStationListDocSize = 2048;
constexpr uint32_t jsonNetworkListDocSize = 256;
constexpr uint32_t jsonFuelStationListDocSize = 1024;

// nextion upload
constexpr size_t segmentSize = 4096;
constexpr uint32_t uploadSpeed = 57600;

// ntp server
constexpr char ntpServer[] = "fritz.box";
constexpr unsigned long ntpUpdateInterval = (123UL);    // in seconds (~2min)
//constexpr char ntpServer[] = "de.pool.ntp.org";
//constexpr unsigned long ntpUpdateInterval = (987UL);    // in seconds (~17min)
// https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
constexpr char ntpTimeszone[] = "CET-1CEST,M3.5.0/02,M10.5.0/03";

// HMI et al
constexpr uint8_t numberOfStationKeys = 6;
constexpr int32_t nextionBufferSize = 32;
constexpr int8_t  numberOfDebugLines = 8;
constexpr uint8_t vs1053MaxVolume = 21;
constexpr uint8_t defaultVolume = 16; // Headphones: 5-8  amplifier connected: 12-18
constexpr int32_t brightnessInterval = 5;
constexpr int32_t brightnessWhenOff = 10;
constexpr int32_t titleTextLength = 96;

enum class Pages { DEBUG, PLAYER, CLOCK, FUEL, DOWNLOAD };
constexpr Pages   startPage = Pages::CLOCK;


// hardware
//    MP3 board
constexpr uint8_t spiMOSI    = 23;
constexpr uint8_t spiMISO    = 19;
constexpr uint8_t spiSCK     = 18;
constexpr uint8_t vs1053CS   = 2;
constexpr uint8_t vs1053DCS  = 4;
constexpr uint8_t vs1053DREQ = 36;

//    Nextion display
constexpr uint8_t nextionRXD = 16;
constexpr uint8_t nextionTXD = 17;

//    Encoder
constexpr uint8_t encCLK    = 25;  // GPIO25   Rotary encoder CLK
constexpr uint8_t encDT     = 26;  // GPIO26   Rotary encoder DT
constexpr uint8_t encSW     = 27;  // GPIO27   Rotary encoder SW
constexpr unsigned long encoderSwitchDebounceInterval = 5;          // milliseconds
constexpr unsigned long encoderSwitchLongPressInterval = (2000);    // milliseconds

// Tankerkoenig
constexpr unsigned long fuelUpdateInterval = 678UL;    // in seconds (~11.3min)
constexpr size_t alarmTextLength = 256;
constexpr int32_t initialLimitDiesel   = 180;
constexpr int32_t initialLimitSuper    = 190;
constexpr int32_t initialLimitSuperE10 = 187;
constexpr uint8_t fuelScanStartHour =   6;   // starts at   ">="
constexpr uint8_t fuelScanEndHour   =  22;   // ends before "<"
constexpr bool enableFuelPriceScanWhileOff = true;
constexpr bool enableSpeechOutput = true;
constexpr bool UseTankerkoenigFakeValues = false;


//=====================================================================================================
