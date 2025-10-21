#pragma once
#include <string>
#include <functional>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

class SDLWrapper {
public:
    SDLWrapper();
    ~SDLWrapper();

    bool init();
    void close();

    bool playAudio(const std::string& filePath);
    void pauseAudio(); // This will toggle pause/resume
    void stopAudio();

    /**
     * @brief Gets the current playback position in seconds.
     * @return Time in seconds, or 0 if not playing.
     */
    int getCurrentTime() const;

    /**
     * @brief Sets a callback function to be called when music finishes.
     * @param callback The function to call.
     */
    void setTrackFinishedCallback(std::function<void()> callback);

private:
    /**
     * @brief Static C-style callback that SDL_mixer will call.
     */
    static void musicFinishedCallback();

    bool isInitialized;
    Mix_Music* currentMusic;
    
    // Static callback pointer to our C++ function
    static std::function<void()> s_onTrackFinishedCallback;
};