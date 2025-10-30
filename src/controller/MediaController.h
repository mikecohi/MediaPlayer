#pragma once
#include "model/MediaManager.h"
#include "model/MediaPlayer.h"
#include "model/MediaFile.h" // Include MediaFile for parameters/return types

#include "utils/DeviceConnector.h"
#include "utils/TagLibWrapper.h"

class Playlist;
class Metadata;

/**
 * @class MediaController
 * @brief Handles user input related to media playback and library management.
 */
class MediaController {
public:
    /**
     * @brief Constructor using Dependency Injection.
     * @param manager Non-owning pointer to MediaManager.
     * @param player Non-owning pointer to MediaPlayer.
     * @param tagUtil Non-owning pointer to TagLibWrapper.
     * @param device Non-owning pointer to DeviceConnector.
     */
    MediaController(MediaManager* manager, MediaPlayer* player, 
                    TagLibWrapper* tagUtil, DeviceConnector* device);

    // --- Methods called from View (User Input) ---
    void playTrack(MediaFile* file); //add remove playlist context
    void pauseOrResume();
    void stop();
    void setVolume(int volume);
    //bool editMetadata(MediaFile* file, const std::string& key, const std::string& value);
    void loadMediaFromPath(const std::string& path);
    bool saveMetadataChanges(MediaFile* file);

    // --- Playback Control Methods ---
    void nextTrack();
    void previousTrack();
    void increaseVolume(int amount = 5); // Increase by a step
    void decreaseVolume(int amount = 5); // Decrease by a step
    void setUSBMediaManager(MediaManager* usbMgr);
    // --- Methods called from S32K144 (Device Input) ---
    void onDevicePlayPause();
    void onDeviceNext();
    void onDevicePrevious();
    void onDeviceVolumeChange(int adcValue); // Raw value from ADC
    // void setUSBMediaManager(MediaManager* mgr);
    void playPlaylist(Playlist* playlist, int startIndex = 0);
private:
    /**
     * @brief Sends current song info to the S32K144 board.
     * @param file The file that just started playing.
     */
    void sendSongInfoToDevice(MediaFile* file);
    
    /**
     * @brief Converts the raw ADC value to a 0-100 volume scale.
     */
    int convertAdcToVolume(int adcValue);

    // Non-owning pointers to models and utilities
    MediaManager* mediaManager;
    MediaManager* usbMediaManager = nullptr;  // thêm dòng này
    MediaPlayer* mediaPlayer;
    MediaFile* findAdjacentTrack(int offset); 
    TagLibWrapper* tagUtil;
    DeviceConnector* deviceConnector;
};