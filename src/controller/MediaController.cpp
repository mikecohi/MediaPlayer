#include "controller/MediaController.h"
#include <iostream> // For logging

// ===================================
// ĐỊNH NGHĨA HÀM TẠO (CONSTRUCTOR)
// Đây là phần mà linker đang thiếu
// ===================================
MediaController::MediaController(MediaManager* manager, MediaPlayer* player, 
                                 TagLibWrapper* tagUtil, DeviceConnector* device)
    : mediaManager(manager), 
      mediaPlayer(player), 
      tagUtil(tagUtil), 
      deviceConnector(device) 
{
    // Constructor body (có thể rỗng ở giai đoạn này)
    if (!manager || !player || !tagUtil || !device) {
        std::cerr << "CRITICAL: MediaController initialized with null pointers!" << std::endl;
    }
    std::cout << "MediaController: Initialized." << std::endl;
}

// --- Các hàm gọi từ View (User Input) ---
void MediaController::playTrack(MediaFile* file) {
    std::cout << "MediaController: (Skeleton) playTrack called." << std::endl;
    if (mediaPlayer && file) {
        mediaPlayer->play(file);
        // (Giai đoạn sau: sendSongInfoToDevice(file);)
    }
}

void MediaController::pauseOrResume() {
    std::cout << "MediaController: (Skeleton) pauseOrResume called." << std::endl;
    if (mediaPlayer) {
        mediaPlayer->pause(); // Hàm pause của MediaPlayer tự xử lý toggle
    }
}

void MediaController::stop() {
    std::cout << "MediaController: (Skeleton) stop called." << std::endl;
    if (mediaPlayer) {
        mediaPlayer->stop();
    }
}

void MediaController::setVolume(int volume) {
    std::cout << "MediaController: (Skeleton) setVolume called with " << volume << std::endl;
     if (mediaPlayer) {
        mediaPlayer->setVolume(volume);
    }
}

bool MediaController::editMetadata(MediaFile* file, const std::string& key, const std::string& value) {
    std::cout << "MediaController: (Skeleton) editMetadata called." << std::endl;
    // (Logic thật sẽ gọi file->getMetadata()->editField() và tagUtil->writeTags())
    return false; // Chưa làm
}

void MediaController::loadMediaFromPath(const std::string& path) {
    std::cout << "MediaController: (Skeleton) loadMediaFromPath called." << std::endl;
     if (mediaManager) {
        mediaManager->loadFromDirectory(path);
    }
}

// --- Các hàm gọi từ S32K144 (Device Input) ---
void MediaController::onDevicePlayPause() {
    std::cout << "MediaController: (Skeleton) onDevicePlayPause received." << std::endl;
    pauseOrResume();
}

void MediaController::onDeviceNext() {
     std::cout << "MediaController: (Skeleton) onDeviceNext received." << std::endl;
     // (Logic thật)
}

void MediaController::onDevicePrevious() {
     std::cout << "MediaController: (Skeleton) onDevicePrevious received." << std::endl;
     // (Logic thật)
}

void MediaController::onDeviceVolumeChange(int adcValue) {
     std::cout << "MediaController: (Skeleton) onDeviceVolumeChange received: " << adcValue << std::endl;
     // int volume = convertAdcToVolume(adcValue);
     // setVolume(volume);
}

// --- Các hàm Private ---
void MediaController::sendSongInfoToDevice(MediaFile* file) {
    // (Logic thật)
}

int MediaController::convertAdcToVolume(int adcValue) {
    // (Logic thật)
    return 50; // Giả lập
}