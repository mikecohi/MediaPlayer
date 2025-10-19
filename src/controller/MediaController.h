#ifndef MEDIA_CONTROLLER_H
#define MEDIA_CONTROLLER_H

#include "../model/MediaFile.h"
#include <iostream>

// Stub for MediaController
// Simulates playing media files for PlaylistController
class MediaController {
public:
    MediaController() {}

    // Mock play function to simulate playback
    void play(const MediaFile& file) {
        std::cout << "[Mock] Playing: " << file.getFileName() << std::endl;
    }

    void pause() {
        std::cout << "[Mock] Paused current track." << std::endl;
    }

    void stop() {
        std::cout << "[Mock] Stopped playback." << std::endl;
    }
};

#endif
