#pragma once
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

/**
 * @class SDLWrapper
 * @brief A wrapper class for the SDL2 and SDL2_mixer libraries.
 * Handles initialization, shutdown, and basic audio playback controls.
 */
class SDLWrapper {
public:
    SDLWrapper();
    ~SDLWrapper();

    /**
     * @brief Initializes the SDL Audio subsystem and SDL_mixer.
     * @return true on success, false on failure.
     */
    bool init();

    /**
     * @brief Shuts down SDL_mixer and SDL.
     */
    void close();

    /**
     * @brief Loads and plays a music file.
     * This is non-blocking; playback happens on SDL_mixer's internal thread.
     * @param filePath The path to the (e.g., MP3, OGG, FLAC) file.
     * @return true on success, false on failure.
     */
    bool playAudio(const std::string& filePath);

    /**
     * @brief Toggles between pause and resume for the current music.
     */
    void pauseAudio();

    /**
     * @brief Stops all music playback and frees the current music resource.
     */
    void stopAudio();

private:
    bool isInitialized;
    Mix_Music* currentMusic; // The currently loaded music object
};