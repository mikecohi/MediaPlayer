#include "controller/AppController.h"

#include "utils/TagLibWrapper.h"
#include "utils/SDLWrapper.h"
#include "utils/DeviceConnector.h"
#include "utils/USBUtils.h"

#include "model/MediaManager.h"
#include "model/PlaylistManager.h"
#include "model/MediaPlayer.h"

#include "controller/MediaController.h"
#include "controller/PlaylistController.h"

#include <iostream>

AppController::AppController() {}
AppController::~AppController() {}

bool AppController::init() {
    // --- 1. Khởi tạo Utils (Không phụ thuộc) ---
    tagLibWrapper = std::make_unique<TagLibWrapper>();
    sdlWrapper = std::make_unique<SDLWrapper>();
    deviceConnector = std::make_unique<DeviceConnector>();
    usbUtils = std::make_unique<USBUtils>();

    // Khởi tạo các hệ thống con
    if (!sdlWrapper->init()) {
        std::cerr << "CRITICAL: Failed to initialize SDLWrapper!" << std::endl;
        return false;
    }
    // (Thêm init cho DeviceConnector... nếu có)

    // --- 2. Khởi tạo Model (Phụ thuộc vào Utils) ---
    mediaPlayer = std::make_unique<MediaPlayer>(sdlWrapper.get());
    mediaManager = std::make_unique<MediaManager>(tagLibWrapper.get());
    playlistManager = std::make_unique<PlaylistManager>(mediaManager.get());

    // --- 3. Khởi tạo Controller (Phụ thuộc vào Model) ---
    mediaController = std::make_unique<MediaController>(
        mediaManager.get(), 
        mediaPlayer.get(), 
        tagLibWrapper.get(), 
        deviceConnector.get()
    );
    playlistController = std::make_unique<PlaylistController>(playlistManager.get());
    
    return true;
}

bool AppController::loadUSBLibrary() {
    if (!usbUtils) return false;

    std::string usbPath = USBUtils::detectUSBPath();
    if (usbPath.empty()) {
        std::cerr << "[AppController] ❌ No USB detected.\n";
        return false;
    }

    currentUSBPath = usbPath;
    std::cout << "[AppController] ✅ USB detected at: " << currentUSBPath << "\n";

    if (mediaManager) {
        mediaManager->loadFromDirectory(currentUSBPath);
        std::cout << "[AppController] ✅ Media library loaded from USB.\n";
        return true;
    }

    return false;
}

bool AppController::reloadUSBLibrary() {
    if (!usbUtils) return false;

    std::cout << "[AppController] 🔁 Reloading USB media library...\n";
    // Trường hợp trong WSL hoặc Linux thực
    if (!currentUSBPath.empty() && USBUtils::isMounted(currentUSBPath)) {
        if (mediaManager) {
            mediaManager->loadFromDirectory(currentUSBPath);
            std::cout << "[AppController] ✅ USB library reloaded successfully.\n";
            return true;
        }
    } else {
        std::cerr << "[AppController] ⚠️ USB is not mounted. Trying to detect again...\n";
        return loadUSBLibrary();
    }
    return false;
}

bool AppController::ejectUSB() {
    if (currentUSBPath.empty() || !usbUtils) return false;

    bool ok = USBUtils::ejectUSB(currentUSBPath);
    if (ok) {
        std::cout << "[AppController] ✅ USB safely ejected.\n";
        currentUSBPath.clear();
        mediaManager->clearLibrary();
    } else {
        std::cerr << "[AppController] ❌ Failed to eject USB.\n";
    }
    return ok;
}




// --- Getters ---
MediaManager* AppController::getMediaManager() const { return mediaManager.get(); }
PlaylistManager* AppController::getPlaylistManager() const { return playlistManager.get(); }
MediaPlayer* AppController::getMediaPlayer() const { return mediaPlayer.get(); }
MediaController* AppController::getMediaController() const { return mediaController.get(); }
PlaylistController* AppController::getPlaylistController() const { return playlistController.get(); }