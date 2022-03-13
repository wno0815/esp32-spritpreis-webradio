
#include "player.h"

// The one and only mp3 player board
VS1053 mp3(vs1053CS, vs1053DCS, vs1053DREQ);

Player::Player() {}

void Player::begin(RadioStations& stationList, RadioStationKeys& keyList)
{
  TRACE();

  stations = stationList;
  keys = keyList;
  memset(currentTitleText, 0, titleTextLength);

  DEB_PF("SPI (%d %d %d)\n", spiSCK, spiMISO, spiMOSI);
  SPI.begin(spiSCK, spiMISO, spiMOSI);
  delay(100);
  DEB_PF("VS1053 (%d %d %d)\n", vs1053CS, vs1053DCS, vs1053DREQ);
  mp3.begin();
  //mp3.printDetails("VS1053 configuration details");
  mp3.printVersion();

  mp3.setVolume(0);
  state = PlayerState::INIT;
}

bool Player::isPlaying()
{
  switch (state)
  {
    case PlayerState::PLAYING:
    case PlayerState::SWITCH:
      return true;
      break;
    case PlayerState::NOT_INIT:
    case PlayerState::INIT:
    case PlayerState::PLAYING_FILE:
    case PlayerState::PLAYING_SPEECH:
    case PlayerState::STOP:
      return false;
      break;
  }
  return false;
}

bool Player::setCurrentStationIndex(int32_t index, int32_t maxIndex)
{
  if ((index >= 0) && (index < maxIndex))
  {
    currentStationIndex = index;
    return true;
  }
  return false;
}

int32_t Player::getCurrentStationIndex()
{
  return currentStationIndex;
}

void Player::setTitleText(const char* newTitleText)
{
  strncpy(currentTitleText, newTitleText, titleTextLength);
}

char* Player::getTitleText()
{
  return currentTitleText;
}

// play a radio station
bool Player::play(int32_t stationToPlay)
{
  TRACE();

  if ((stationToPlay == currentStationIndex) && isPlaying())
  {
    // nothing to do
    // requested station is already playing
    return true;
  }

  switch (state)
  {
    case PlayerState::INIT:
    case PlayerState::STOP:
    case PlayerState::PLAYING:
      nextStationIndex = stationToPlay;
      state = PlayerState::SWITCH;
      break;

    case PlayerState::NOT_INIT:
      // ooops. That should not happen
      DEB_PL("PLAYER: warning: play() called, but not initialized");
      return false;
      break;

    // other cases:
    // should not happen, but not really a problem - don't change something
    case PlayerState::PLAYING_FILE:
    case PlayerState::PLAYING_SPEECH:
    case PlayerState::SWITCH:
      break;
  }

  return true;
}

bool Player::playKey(int32_t key)
{
  TRACE();

  DEB_PF("PLAYER: key: %d   stationIndex: %d\n", key, keys.getStationIndex(key));
  return play(keys.getStationIndex(key));
}

bool Player::playStation(int32_t index)
{
  TRACE();

  DEB_PF("PLAYER: direct play %d '%s'\n", index, stations[index].getName());
  mp3.connecttohost(stations[index].getUrl());
  currentStationIndex = index;
  mp3.setVolume(defaultVolume);
  currentVolume = defaultVolume;
  state = PlayerState::PLAYING;
  stationHasChanged = true;
  return true;
}

bool Player::playFile(const char* fileName)
{
  TRACE();

  DEB_PF("PLAYER: direct play file '%s'\n", fileName);
  if ((state != PlayerState::PLAYING_FILE) && (state != PlayerState::PLAYING_SPEECH))
  {
    lastStateBeforeFileOrSpeech = state;
  }
  stop();
  mp3.connecttoFS(LITTLEFS, fileName);
  mp3.setVolume(defaultVolume);
  currentVolume = defaultVolume;
  state = PlayerState::PLAYING_FILE;
  return true;
}

void Player::setSpeechPending(const bool isPending)
{
  isSpeechPending = isPending;
}

bool Player::getSpeechPending()
{
  return isSpeechPending;
}

bool Player::playSpeech(const char* text)
{
  TRACE();

  DEB_PF("PLAYER: play text '%s'\n", text);
  if ((state != PlayerState::PLAYING_FILE) && (state != PlayerState::PLAYING_SPEECH))
  {
    lastStateBeforeFileOrSpeech = state;
  }
  stop();
  mp3.connecttospeech(text, "de");
  mp3.setVolume(defaultVolume);
  currentVolume = defaultVolume;
  state = PlayerState::PLAYING_SPEECH;
  return true;
}

bool Player::resumeAfterFileOrSpeech()
{
  if (lastStateBeforeFileOrSpeech == PlayerState::PLAYING)
  {
    playStation(currentStationIndex);
    return true;
  }
  return false;
}

bool Player::playNextPrevious(bool next)
{
  TRACE();

  if (!isPlaying())
  {
    // nothing to do
    // player is currently doing nothing
    return false;
  }

  if (next)
  {
    if (currentStationIndex < stations.getNumberOfStations() - 1)
    {
      nextStationIndex = currentStationIndex + 1;
    }
    else
    {
      nextStationIndex = 0;
    }
  }
  else
  {
    if (currentStationIndex == 0)
    {
      nextStationIndex = stations.getNumberOfStations() - 1;
    }
    else
    {
      nextStationIndex = currentStationIndex - 1;
    }
  }
  return play(nextStationIndex);
}

bool Player::stop()
{
  TRACE();

  if (!isPlaying())
  {
    // nothing to do
    // player is currently doing nothing
  }

  switch (state)
  {
    case PlayerState::SWITCH:
    case PlayerState::PLAYING:
    case PlayerState::PLAYING_FILE:
    case PlayerState::PLAYING_SPEECH:
      mp3.setVolume(0);
      currentVolume = 0;
      mp3.stop_mp3client();
      stationHasChanged = true;
      state = PlayerState::STOP;
      break;

    case PlayerState::INIT:
    case PlayerState::STOP:
      DEB_PL("PLAYER: player is already stopoed");
      return false;
      break;

    case PlayerState::NOT_INIT:
      // ooops. That should not happen
      DEB_PL("PLAYER: warning: stop() called, but not initialized");
      return false;
      break;
  }
  return true;
}

bool Player::hasStationChanged()
{
  if (stationHasChanged)
  {
    stationHasChanged = false;
    return true;
  }
  return false;
}


void Player::run()
{
  if (state != lastState)
  {
#if defined(DEBUGGING)
    switch (state)
    {
      case PlayerState::NOT_INIT:
        DEB_PL("PLAYER: NOT_INIT");
        break;
      case PlayerState::INIT:
        DEB_PL("PLAYER: INIT");
        break;
      case PlayerState::PLAYING:
        DEB_PL("PLAYER: PLAYING");
        break;
      case PlayerState::PLAYING_FILE:
        DEB_PL("PLAYER: PLAYING_FILE");
        break;
      case PlayerState::PLAYING_SPEECH:
        DEB_PL("PLAYER: PLAYING_SPEECH");
        break;
      case PlayerState::SWITCH:
        DEB_PL("PLAYER: SWITCH");
        break;
      case PlayerState::STOP:
        DEB_PL("PLAYER: STOP");
        break;
    }
#endif
    lastState = state;
  }

  switch (state)
  {
    case PlayerState::PLAYING:
    case PlayerState::PLAYING_FILE:
    case PlayerState::PLAYING_SPEECH:
      mp3.loop();
      break;
    case PlayerState::SWITCH:
      // mute
      mp3.setVolume(0);
      // switch station
      DEB_PF("PLAYER: switch to station index %d '%s'\n", nextStationIndex, stations[nextStationIndex].getName());

      memset(currentTitleText, 0, titleTextLength);
      if (mp3.connecttohost(stations[nextStationIndex].getUrl()))
      {
        currentStationIndex = nextStationIndex;
        stationHasChanged = true;
      }
      else
      {
        DEB_PF("PLAYER: warning: station switch failed, still playing %d '%s'", currentStationIndex, stations[currentStationIndex].getName());
      }
      // unmute
      mp3.setVolume(currentVolume);
      // play
      state = PlayerState::PLAYING;
      break;

    case PlayerState::NOT_INIT:
    case PlayerState::INIT:
    case PlayerState::STOP:
      break;
  }
}

int32_t Player::getCurrentVolume()
{
  return currentVolume;
}

void Player::changeVolume(bool increment)
{
  if (increment)
  {
    if (currentVolume < vs1053MaxVolume)
    {
      currentVolume++;
    }
  }
  else
  {
    if (currentVolume > 0)
    {
      currentVolume--;
    }
  }
  mp3.setVolume(currentVolume);
}

void Player::setVolume(int32_t volume)
{
  TRACE();

  DEB_PF("PLAYER: current: %d   new: %d\n", currentVolume, volume);
  int32_t volumeToSet = volume < 0 ? 0 : (volume > vs1053MaxVolume ? vs1053MaxVolume : volume);
  mp3.setVolume(volumeToSet);
  currentVolume = volumeToSet;
}
