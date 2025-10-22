#pragma once
#include <memory>
#include "MediaFile.h"
#include "utils/SDLWrapper.h"

enum class PlayerState { STOPPED, PLAYING, PAUSED };


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
    
    //Callback function to be triggered by SDLWrapper when a track finishes.
    void onTrackFinished(); 

private:
    SDLWrapper* sdlWrapper;     
    MediaFile* currentTrack;  
    int currentVolume;

    PlayerState currentState;
};