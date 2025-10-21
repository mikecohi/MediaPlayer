#pragma once
#include <memory> // For std::unique_ptr

// Forward-declare (khai báo trước) tất cả Model, Util, Controller
// (Để tránh include vòng)
class MediaManager;
class PlaylistManager;
class MediaPlayer;
class MediaController;
class PlaylistController;

class TagLibWrapper;
class SDLWrapper;
class DeviceConnector;
class USBUtils;

/**
 * @class AppController
 * @brief The main "brain" of the application.
 * It owns all core Model, Utility, and Controller objects.
 */
class AppController {
public:
    AppController();
    ~AppController(); // Cần thiết để xử lý unique_ptr

    /**
     * @brief Initializes all subsystems (SDL, TagLib, etc.).
     * @return true on success.
     */
    bool init();

    // --- Getters ---
    // Cung cấp các con trỏ (không sở hữu) cho UIManager và các View
    MediaManager* getMediaManager() const;
    PlaylistManager* getPlaylistManager() const;
    MediaPlayer* getMediaPlayer() const;
    MediaController* getMediaController() const;
    PlaylistController* getPlaylistController() const;

private:
    // --- Sở hữu (Ownership) tất cả các Utils ---
    std::unique_ptr<TagLibWrapper> tagLibWrapper;
    std::unique_ptr<SDLWrapper> sdlWrapper;
    std::unique_ptr<DeviceConnector> deviceConnector;
    std::unique_ptr<USBUtils> usbUtils;

    // --- Sở hữu (Ownership) tất cả các Model ---
    std::unique_ptr<MediaManager> mediaManager;
    std::unique_ptr<PlaylistManager> playlistManager;
    std::unique_ptr<MediaPlayer> mediaPlayer;

    // --- Sở hữu (Ownership) tất cả các Controller ---
    std::unique_ptr<MediaController> mediaController;
    std::unique_ptr<PlaylistController> playlistController;
};