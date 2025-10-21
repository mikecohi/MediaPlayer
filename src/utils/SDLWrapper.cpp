#include "SDLWrapper.h"
#include <iostream>

// Initialize the static callback
std::function<void()> SDLWrapper::s_onTrackFinishedCallback = nullptr;

SDLWrapper::SDLWrapper() : isInitialized(false), currentMusic(nullptr) {}

SDLWrapper::~SDLWrapper() {
    close();
}

// Static function for SDL_mixer to call
void SDLWrapper::musicFinishedCallback() {
    if (s_onTrackFinishedCallback) {
        s_onTrackFinishedCallback(); // Call the C++ function
    }
}

bool SDLWrapper::init() {
    if (isInitialized) return true;
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDLWrapper: Could not initialize SDL. " << SDL_GetError() << std::endl;
        return false;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDLWrapper: Could not initialize SDL_mixer. " << Mix_GetError() << std::endl;
        SDL_Quit();
        return false;
    }
    Mix_AllocateChannels(16);

    // Register our static function with SDL_mixer
    Mix_HookMusicFinished(SDLWrapper::musicFinishedCallback);

    isInitialized = true;
    return true;
}

void SDLWrapper::close() {
    if (!isInitialized) return;
    stopAudio();
    Mix_HookMusicFinished(nullptr); // Unregister callback
    Mix_CloseAudio();
    while(Mix_QuerySpec(nullptr, nullptr, nullptr)) {}
    Mix_Quit();
    SDL_Quit();
    isInitialized = false;
}

bool SDLWrapper::playAudio(const std::string& filePath) {
    if (!isInitialized) return false;
    stopAudio();
    currentMusic = Mix_LoadMUS(filePath.c_str());
    if (currentMusic == nullptr) {
        std::cerr << "SDLWrapper: Failed to load music: " << Mix_GetError() << std::endl;
        return false;
    }
    if (Mix_PlayMusic(currentMusic, 1) == -1) { // Play 1 time
        std::cerr << "SDLWrapper: Failed to play music: " << Mix_GetError() << std::endl;
        Mix_FreeMusic(currentMusic);
        currentMusic = nullptr;
        return false;
    }
    return true;
}

void SDLWrapper::pauseAudio() {
    if (Mix_PlayingMusic()) {
        if (Mix_PausedMusic()) Mix_ResumeMusic();
        else Mix_PauseMusic();
    }
}

void SDLWrapper::stopAudio() {
    Mix_HaltMusic();
    if (currentMusic != nullptr) {
        Mix_FreeMusic(currentMusic);
        currentMusic = nullptr;
    }
}

// --- HÀM MỚI ---
int SDLWrapper::getCurrentTime() const {
    if (Mix_PlayingMusic()) {
        // Mix_GetMusicPosition returns seconds as a double
        return static_cast<int>(Mix_GetMusicPosition(currentMusic));
    }
    return 0;
}

// --- HÀM MỚI ---
void SDLWrapper::setTrackFinishedCallback(std::function<void()> callback) {
    s_onTrackFinishedCallback = callback;
}