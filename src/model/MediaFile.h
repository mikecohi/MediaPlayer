#pragma once
#include <string>
#include <memory> // For std::unique_ptr
#include "Metadata.h"
#include "AudioMetadata.h" // Need definitions for dynamic_cast
#include "VideoMetadata.h" // in .cpp

// Enum to identify media type
enum class MediaType { UNKNOWN, AUDIO, VIDEO };

/**
 * @class MediaFile
 * @brief Represents a single media file on disk.
 * It owns its Metadata object via std::unique_ptr for automatic memory management.
 */
class MediaFile {
public:
    /**
     * @brief Constructor.
     * @param path The full path to the file.
     * @param metadata A unique_ptr to the polymorphic Metadata object.
     */
    MediaFile(const std::string& path, std::unique_ptr<Metadata> metadata);

    const std::string& getFilePath() const;
    const std::string& getFileName() const; // For display
    MediaType getType() const;
    Metadata* getMetadata() const; // Returns a raw pointer for viewing/editing

private:
    std::string filePath;
    std::string fileName; // Extracted from filePath
    MediaType mediaType;
    std::unique_ptr<Metadata> metadata; // Owning smart pointer
};