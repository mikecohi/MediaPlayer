#pragma once
#include "Metadata.h"

/**
 * @class VideoMetadata
 * @brief Inherits from Metadata and adds video-specific fields.
 */
class VideoMetadata : public Metadata {
public:
    std::string resolution; // e.g., "1920x1080"
    int bitrateKbps;    // e.g., 3000
    std::string codec;      // e.g., "h264"
};