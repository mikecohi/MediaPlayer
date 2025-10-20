#pragma once
#include <memory>
#include "model/MediaManager.h"
#include "model/PlaylistManager.h"
#include "model/MediaPlayer.h"
#include "controller/MediaController.h"
#include "controller/PlaylistController.h"
// Include all utility headers
#include "utils/FileUtils.h"
#include "utils/TagLibWrapper.h"
#include "utils/SDLWrapper.h"
#include "utils/DeviceConnector.h"
#include "utils/USBUtils.h"

// Forward-declare ViewManager to avoid circular include dependencies
class ViewManager;

/**
 * @class AppController
 * @brief The main "orchestrator" class.
 * It owns all Model, Controller, and Utility objects.
 * It manages the main application loop.
 */
class AppController {
public:
    AppController(ViewManager* viewManager);
    ~AppController(); // Required for cleanup of unique_ptrs

    void initialize(); // Initialize all subsystems
    void run();        // Start the main application loop
    void shutdown();   // Clean up and exit

    // --- Getters for ViewManager ---
    // Provides non-owning pointers to the View layer for rendering data.
    MediaManager* getMediaManager();
    PlaylistManager* getPlaylistManager();
    MediaPlayer* getMediaPlayer();
    MediaController* getMediaController();
    PlaylistController* getPlaylistController();

private:
    /**
     * @brief Processes input from keyboard/mouse via Ncurses.
     */
    void processUserInput();

    /**
     * @brief Processes input from the S32K144 board.
     */
    void processDeviceInput();

    ViewManager* viewManager; // Non-owning pointer

    // --- Ownership of all Utility objects ---
    std::unique_ptr<FileUtils> fileUtils;
    std::unique_ptr<TagLibWrapper> tagLibWrapper;
    std::unique_ptr<SDLWrapper> sdlWrapper;
    std::unique_ptr<DeviceConnector> deviceConnector;
    std::unique_ptr<USBUtils> usbUtils;

    // --- Ownership of all Model objects ---
    std::unique_ptr<MediaManager> mediaManager;
    std::unique_ptr<PlaylistManager> playlistManager;
    std::unique_ptr<MediaPlayer> mediaPlayer;

    // --- Ownership of all Controller objects ---
    std::unique_ptr<MediaController> mediaController;
    std::unique_ptr<PlaylistController> playlistController;
    
    bool isRunning;
};