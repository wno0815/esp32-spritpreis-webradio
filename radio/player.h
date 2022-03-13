#pragma once
#include "Arduino.h"

#include "trace.h"
#include "config.h"

#include "WiFi.h"
#include "SPI.h"
#include "vs1053_ext.h"
#include "storage.h"
#include "station.h"

class Player
{
  public:
    Player();

    void begin(RadioStations& stationList, RadioStationKeys& keyList);
    bool isPlaying();
    bool play(int32_t stationToPlay = -1);
    bool playKey(int32_t stationKey);
    bool playStation(int32_t stationIndex);
    bool playFile(const char* fileName = "gong.mp3");
    bool playSpeech(const char* text);
    void setSpeechPending(const bool isPending);
    bool getSpeechPending();
    bool resumeAfterFileOrSpeech();
    bool playNextPrevious(bool next);
    void run();
    bool stop();
    bool setCurrentStationIndex(int32_t index, int32_t maxIndex = 0);
    int32_t getCurrentStationIndex();
    bool hasStationChanged();
    void setTitleText(const char* newTitleText);
    char* getTitleText();

    int32_t getCurrentVolume();
    void setVolume(int32_t volume);
    void changeVolume(bool increment);

  private:
    enum class PlayerState { NOT_INIT, INIT, PLAYING, PLAYING_FILE, PLAYING_SPEECH, SWITCH, STOP };

    int32_t currentStationIndex = -1;
    bool stationHasChanged = false;
    int32_t nextStationIndex;
    RadioStations stations;
    RadioStationKeys keys;
    uint8_t currentVolume = 0;
    PlayerState state = PlayerState::NOT_INIT;
    PlayerState nextState = PlayerState::STOP;
    PlayerState lastState = PlayerState::NOT_INIT;
    PlayerState lastStateBeforeFileOrSpeech = PlayerState::NOT_INIT;
    char currentTitleText[titleTextLength+1];
    bool isSpeechPending = false;
};
