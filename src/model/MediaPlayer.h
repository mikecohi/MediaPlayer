#pragma once
#include <memory>
#include "MediaFile.h"
#include "utils/SDLWrapper.h"

// Enum to represent the player's current state
enum class PlayerState { STOPPED, PLAYING, PAUSED };

/**
 * @class MediaPlayer
 * @brief Manages the state of media playback (play, pause, volume).
 */
class MediaPlayer {
public:
    MediaPlayer(SDLWrapper* sdlWrapper);

    void play(MediaFile* file);
    void pause(); // Toggles pause/resume
    void stop();

    void setVolume(int volume); // 0-100
    int getVolume() const;

    PlayerState getState() const;
    int getCurrentTime() const; // Gets current time from SDLWrapper
    int getTotalTime() const;   // Gets total duration from MediaFile
    MediaFile* getCurrentTrack() const;
    
    /**
     * @brief A callback function to be triggered by SDLWrapper when a track finishes.
     */
    void onTrackFinished(); 

private:
    SDLWrapper* sdlWrapper;     // Non-owning pointer
    MediaFile* currentTrack;    // Non-owning pointer
    PlayerState currentState;
    int currentVolume;
};