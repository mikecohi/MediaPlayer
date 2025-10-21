#include "model/MediaPlayer.h"
#include <iostream>

MediaPlayer::MediaPlayer(SDLWrapper* sdlWrapper)
    : sdlWrapper(sdlWrapper), 
      currentTrack(nullptr), 
      currentState(PlayerState::STOPPED), 
      currentVolume(100) // Default volume
{
    if (sdlWrapper == nullptr) {
        std::cerr << "CRITICAL: MediaPlayer started with null SDLWrapper!" << std::endl;
        return;
    }

    // Connect the SDLWrapper's static callback to our member function
    // We use a lambda to capture 'this'
    sdlWrapper->setTrackFinishedCallback([this]() {
        this->onTrackFinished();
    });
}

void MediaPlayer::play(MediaFile* file) {
    if (file == nullptr) return;

    // (Giai đoạn 3 sẽ kiểm tra xem có phải file đang pause không)
    
    if (sdlWrapper->playAudio(file->getFilePath())) {
        currentTrack = file;
        currentState = PlayerState::PLAYING;
    } else {
        currentTrack = nullptr;
        currentState = PlayerState::STOPPED;
    }
}

void MediaPlayer::pause() {
    if (currentState == PlayerState::PLAYING) {
        sdlWrapper->pauseAudio(); // Pause
        currentState = PlayerState::PAUSED;
    } else if (currentState == PlayerState::PAUSED) {
        sdlWrapper->pauseAudio(); // Resume
        currentState = PlayerState::PLAYING;
    }
}

void MediaPlayer::stop() {
    sdlWrapper->stopAudio();
    currentState = PlayerState::STOPPED;
    currentTrack = nullptr;
}

void MediaPlayer::setVolume(int volume) {
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;
    currentVolume = volume;
    
    // SDL_mixer volume is 0-128
    int sdlVolume = (currentVolume * MIX_MAX_VOLUME) / 100;
    Mix_VolumeMusic(sdlVolume);
}

int MediaPlayer::getVolume() const {
    return currentVolume;
}

PlayerState MediaPlayer::getState() const {
    return currentState;
}

int MediaPlayer::getCurrentTime() const {
    if (currentState == PlayerState::STOPPED) {
        return 0;
    }
    return sdlWrapper->getCurrentTime();
}

int MediaPlayer::getTotalTime() const {
    if (currentTrack) {
        Metadata* meta = currentTrack->getMetadata();
        if (meta) {
            return meta->durationInSeconds;
        }
    }
    return 0;
}

MediaFile* MediaPlayer::getCurrentTrack() const {
    return currentTrack;
}

void MediaPlayer::onTrackFinished() {
    std::cout << "MediaPlayer: Track finished." << std::endl;
    currentState = PlayerState::STOPPED;
    currentTrack = nullptr;
    // (Giai đoạn 3: Logic auto-next sẽ được thêm vào đây)
}