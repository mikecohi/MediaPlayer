#include "controller/MediaController.h"
#include "model/MediaFile.h" // Needed for file operations
#include "model/Metadata.h"  // Needed for editing
#include <iostream> 

MediaController::MediaController(MediaManager* manager, MediaPlayer* player, 
                                 TagLibWrapper* tagUtil, DeviceConnector* device)
    : mediaManager(manager), 
      mediaPlayer(player), 
      tagUtil(tagUtil), 
      deviceConnector(device) 
{
    if (!manager || !player || !tagUtil || !device) {
        std::cerr << "CRITICAL: MediaController initialized with null pointers!" << std::endl;
    }
    std::cout << "MediaController: Initialized." << std::endl;
}

// --- Các hàm gọi từ View (User Input) ---

void MediaController::playTrack(MediaFile* file) {
    std::cout << "MediaController: playTrack called for " << (file ? file->getFileName() : "nullptr") << std::endl;
    if (mediaPlayer && file) {
        mediaPlayer->play(file);
        // Giai đoạn sau: sendSongInfoToDevice(file);
    } else {
        std::cerr << "MediaController: Cannot play track (null player or file)." << std::endl;
    }
}

void MediaController::pauseOrResume() {
    std::cout << "MediaController: pauseOrResume called." << std::endl;
    if (mediaPlayer) {
        mediaPlayer->pause(); // MediaPlayer handles the toggle logic
    }
}

void MediaController::stop() {
    std::cout << "MediaController: stop called." << std::endl;
    if (mediaPlayer) {
        mediaPlayer->stop();
    }
}

void MediaController::setVolume(int volume) {
    std::cout << "MediaController: setVolume called with " << volume << std::endl;
     if (mediaPlayer) {
        mediaPlayer->setVolume(volume);
    }
}

bool MediaController::editMetadata(MediaFile* file, const std::string& key, const std::string& value) {
    std::cout << "MediaController: editMetadata called for " << key << "=" << value << std::endl;
    if (!file || !file->getMetadata() || !tagUtil) {
        std::cerr << "MediaController: Cannot edit metadata (null pointers)." << std::endl;
        return false;
    }
    
    // 1. Update the metadata object in memory
    Metadata* meta = file->getMetadata();
    meta->editField(key, value); 
    
    // 2. Write the changes back to the actual file on disk
    // (Lưu ý: Cần kiểm tra TagLibWrapper::writeTags được triển khai chưa)
    // bool success = tagUtil->writeTags(file->getFilePath(), *meta);
    // if (!success) {
    //    std::cerr << "MediaController: Failed to write tags to file: " << file->getFilePath() << std::endl;
    //    // Optionally revert the change in memory?
    // }
    // return success;
    
    std::cout << "MediaController: writeTags not fully implemented yet." << std::endl;
    return true; // Tạm thời trả về true
}

void MediaController::loadMediaFromPath(const std::string& path) {
    std::cout << "MediaController: loadMediaFromPath called for " << path << std::endl;
     if (mediaManager) {
        mediaManager->loadFromDirectory(path);
    }
}

// --- Các hàm gọi từ S32K144 (Device Input) ---
// (Các hàm này đã có skeleton, không cần sửa)
void MediaController::onDevicePlayPause() {
    std::cout << "MediaController: onDevicePlayPause received." << std::endl;
    pauseOrResume();
}
// ... (onDeviceNext, onDevicePrevious, onDeviceVolumeChange giữ nguyên skeleton) ...
void MediaController::onDeviceNext() { std::cout << "MediaController: (Stub) onDeviceNext.\n"; }
void MediaController::onDevicePrevious() { std::cout << "MediaController: (Stub) onDevicePrevious.\n"; }
void MediaController::onDeviceVolumeChange(int adcValue) { std::cout << "MediaController: (Stub) onDeviceVolumeChange: " << adcValue << "\n"; }

// --- Các hàm Private ---
// (Giữ nguyên skeleton)
void MediaController::sendSongInfoToDevice(MediaFile* file) {}
int MediaController::convertAdcToVolume(int adcValue) { return 50; }