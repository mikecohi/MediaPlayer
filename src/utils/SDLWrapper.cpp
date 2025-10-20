#include "SDLWrapper.h"
#include <iostream>

SDLWrapper::SDLWrapper() : isInitialized(false), currentMusic(nullptr) {
    // Constructor
}

SDLWrapper::~SDLWrapper() {
    close();
}

bool SDLWrapper::init() {
    if (isInitialized) {
        return true;
    }

    // Initialize SDL's Audio subsystem
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDLWrapper Error: Could not initialize SDL. " << SDL_GetError() << std::endl;
        return false;
    }

    // Initialize SDL_mixer
    // 44100 Hz, default format, 2 channels (stereo), 2048 chunk size
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDLWrapper Error: Could not initialize SDL_mixer. " << Mix_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    // Allocate 16 mixing channels
    Mix_AllocateChannels(16);

    isInitialized = true;
    std::cout << "SDLWrapper: Initialized successfully." << std::endl;
    return true;
}

void SDLWrapper::close() {
    if (!isInitialized) {
        return;
    }
    
    stopAudio();

    // Shut down SDL_mixer
    Mix_CloseAudio();
    while(Mix_QuerySpec(nullptr, nullptr, nullptr)) {} // Wait for audio to close
    Mix_Quit();

    // Shut down SDL
    SDL_Quit();
    
    isInitialized = false;
    std::cout << "SDLWrapper: Shut down." << std::endl;
}

bool SDLWrapper::playAudio(const std::string& filePath) {
    if (!isInitialized) {
        std::cerr << "SDLWrapper Error: Must call init() before playAudio()." << std::endl;
        return false;
    }

    // Stop and free any music that is already playing
    stopAudio();

    // Load the new music file
    currentMusic = Mix_LoadMUS(filePath.c_str());
    if (currentMusic == nullptr) {
        std::cerr << "SDLWrapper Error: Failed to load music: " << Mix_GetError() << std::endl;
        return false;
    }

    // Play the music (play 1 time)
    // Mix_PlayMusic handles this on its own thread.
    if (Mix_PlayMusic(currentMusic, 1) == -1) {
        std::cerr << "SDLWrapper Error: Failed to play music: " << Mix_GetError() << std::endl;
        Mix_FreeMusic(currentMusic); // Free the failed music
        currentMusic = nullptr;
        return false;
    }

    return true;
}

void SDLWrapper::pauseAudio() {
    if (!isInitialized) {
        return;
    }

    // Check if music is playing (and not just paused)
    if (Mix_PlayingMusic()) {
        if (Mix_PausedMusic()) {
            Mix_ResumeMusic(); // It was paused, so resume
        } else {
            Mix_PauseMusic(); // It was playing, so pause
        }
    }
}

void SDLWrapper::stopAudio() {
    if (!isInitialized) {
        return;
    }

    // Stop all music
    Mix_HaltMusic();

    // Free the music chunk if it exists
    if (currentMusic != nullptr) {
        Mix_FreeMusic(currentMusic);
        currentMusic = nullptr;
    }
}