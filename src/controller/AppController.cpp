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
#include <filesystem>
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
    usbMediaManager = std::make_unique<MediaManager>(tagLibWrapper.get());

    playlistManager = std::make_unique<PlaylistManager>(mediaManager.get());
    playlistManager->setUSBMediaManager(usbMediaManager.get());

    // --- 3. Khởi tạo Controller (Phụ thuộc vào Model) ---
    mediaController = std::make_unique<MediaController>(
        mediaManager.get(), 
        mediaPlayer.get(), 
        tagLibWrapper.get(), 
        deviceConnector.get()
    );
    playlistController = std::make_unique<PlaylistController>(playlistManager.get());

    mediaPlayer->setOnTrackFinishedCallback([this]() {
        // [this] capture con trỏ AppController
        // Chúng ta cần kiểm tra mediaController tồn tại trước khi gọi
        if (this->mediaController) {
            this->mediaController->nextTrack();
        }
    });
    return true;
}

bool AppController::loadUSBLibrary() {
    if (!usbUtils || !usbMediaManager) return false;

    currentUSBPath = usbUtils->detectUSBMount();
    if (currentUSBPath.empty()) {
        std::cerr << "[AppController] ❌ No USB detected.\n";
        return false;
    }

    std::cout << "[AppController] ✅ Loading media from: " << currentUSBPath << std::endl;
    usbMediaManager->loadFromDirectory(currentUSBPath);

    
    // 🔹 Sau khi load USB, cập nhật lại PlaylistManager
    if (playlistManager)
        playlistManager->setUSBMediaManager(usbMediaManager.get());
        
    return true;
}

bool AppController::reloadUSBLibrary() {
    std::cout << "[AppController] 🔄 Reloading USB library..." << std::endl;

    if (!usbUtils) return false;
    std::string newPath = usbUtils->detectUSBMount();

    if (newPath.empty() || !std::filesystem::exists(newPath)) {
        std::cerr << "[AppController] ⚠️ No valid USB path to reload.\n";
        return false;
    }

    currentUSBPath = newPath;
    if (usbMediaManager) {
        usbMediaManager->loadFromDirectory(currentUSBPath);
    }
    return true;
}

bool AppController::ejectUSB() {
    if (!usbUtils) return false;

    if (currentUSBPath.empty()) {
        std::cerr << "[AppController] ⚠️ No USB currently mounted.\n";
        return false;
    }

    // --- 1️⃣ Chỉ dừng phát nếu bài hiện tại nằm trong USB ---
    if (mediaPlayer) {
        MediaFile* current = mediaPlayer->getCurrentTrack();
        if (current) {
            std::string currentPath = current->getFilePath();
            // Kiểm tra xem bài đang phát có nằm trong USB mount path hay không
            if (currentPath.rfind(currentUSBPath, 0) == 0) {
                std::cout << "[AppController] ⏹ Stopping track from USB before eject.\n";
                mediaPlayer->stop();
            } else {
                std::cout << "[AppController] ▶ Current track not from USB. Continue playing.\n";
            }
        }
    }

    // --- 2️⃣ Thực hiện eject ---
    bool ok = usbUtils->unmountUSB(currentUSBPath);
    if (ok && usbMediaManager) {
        usbMediaManager->clearLibrary(); // 🔹 clear current data
        std::cout << "[AppController] 🧹 USB MediaManager cleared after eject.\n";
    }
    currentUSBPath.clear();
    return ok;
}



// --- Getters ---
MediaManager* AppController::getMediaManager() const { return mediaManager.get(); }
PlaylistManager* AppController::getPlaylistManager() const { return playlistManager.get(); }
MediaPlayer* AppController::getMediaPlayer() const { return mediaPlayer.get(); }
MediaController* AppController::getMediaController() const { return mediaController.get(); }
PlaylistController* AppController::getPlaylistController() const { return playlistController.get(); }
MediaManager* AppController::getUSBMediaManager() const {return usbMediaManager.get(); }