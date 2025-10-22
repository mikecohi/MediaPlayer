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

// --- Next/Previous ---
MediaFile* MediaController::findAdjacentTrack(int offset) {
    if (!mediaPlayer || !mediaManager) return nullptr;

    MediaFile* current = mediaPlayer->getCurrentTrack();
    if (!current) return nullptr; // Cannot determine next/prev if nothing is playing

    // A simple, potentially inefficient way for now: Get all files
    // A better way would involve knowing the current playlist or view context
    // For now, assume we operate on the entire library page by page implicitly
    int totalFiles = mediaManager->getTotalFileCount();
    if (totalFiles <= 1) return nullptr; // Only one song

    // Find the index of the current track (VERY INEFFICIENT for large libraries!)
    int currentIndex = -1;
    // We have to iterate through pages to find the index
    int pageSize = 25; // Assume standard page size for now
    int totalPages = mediaManager->getTotalPages(pageSize);
    for (int p = 1; p <= totalPages; ++p) {
        std::vector<MediaFile*> page = mediaManager->getPage(p, pageSize);
        for(size_t i = 0; i < page.size(); ++i) {
            if (page[i] == current) {
                currentIndex = (p - 1) * pageSize + i;
                goto found_index; // Exit nested loops
            }
        }
    }

found_index:
    if (currentIndex == -1) return nullptr; // Current track not found? Should not happen

    // Calculate the next/previous index with wrapping
    int nextIndex = (currentIndex + offset + totalFiles) % totalFiles;

    // Get the MediaFile* at the new index (again, inefficiently)
    int targetPage = (nextIndex / pageSize) + 1;
    int indexOnPage = nextIndex % pageSize;
    std::vector<MediaFile*> targetPageData = mediaManager->getPage(targetPage, pageSize);

    if (indexOnPage >= 0 && static_cast<size_t>(indexOnPage) < targetPageData.size()) {
        return targetPageData[indexOnPage];
    }

    return nullptr; // Error finding the adjacent track
}


// --- Playback Control Implementations ---
void MediaController::nextTrack() {
    std::cout << "MediaController: nextTrack called." << std::endl;
    MediaFile* next = findAdjacentTrack(1); // Get the next track (+1 offset)
    if (next) {
        playTrack(next); // Play it
    } else {
        std::cout << "MediaController: Could not find next track." << std::endl;
        // Optional: Stop playback if at the end? mediaPlayer->stop();
    }
}

void MediaController::previousTrack() {
    std::cout << "MediaController: previousTrack called." << std::endl;
    MediaFile* prev = findAdjacentTrack(-1); // Get the previous track (-1 offset)
    if (prev) {
        playTrack(prev); // Play it
    } else {
        std::cout << "MediaController: Could not find previous track." << std::endl;
        // Optional: Stop playback or restart current? mediaPlayer->stop();
    }
}

void MediaController::increaseVolume(int amount) {
    std::cout << "MediaController: increaseVolume called." << std::endl;
    if (mediaPlayer) {
        int currentVol = mediaPlayer->getVolume();
        setVolume(std::min(100, currentVol + amount)); // Use setVolume, clamp at 100
    }
}

void MediaController::decreaseVolume(int amount) {
    std::cout << "MediaController: decreaseVolume called." << std::endl;
    if (mediaPlayer) {
        int currentVol = mediaPlayer->getVolume();
        setVolume(std::max(0, currentVol - amount)); // Use setVolume, clamp at 0
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