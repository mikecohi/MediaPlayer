#include "TagLibWrapper.h"
#include "model/AudioMetadata.h" // We will create AudioMetadata objects
#include "model/VideoMetadata.h" // (For future use)
#include <iostream>
#include <filesystem> // For getting filename as fallback

// TagLib Includes
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>
#include <taglib/audioproperties.h>

TagLibWrapper::TagLibWrapper() {
    // Constructor (can be empty)
}

TagLibWrapper::~TagLibWrapper() {
    // Destructor (can be empty)
}

std::unique_ptr<Metadata> TagLibWrapper::readTags(const std::string& filePath) {
    TagLib::FileRef f(filePath.c_str());

    if (f.isNull() || !f.tag()) {
        std::cerr << "TagLibWrapper: Could not read file or tags for " << filePath << std::endl;
        return nullptr;
    }

    TagLib::Tag* tag = f.tag();
    TagLib::AudioProperties* props = f.audioProperties();

    // For Phase 1, we will focus on files with audio properties.
    // TagLib has limited support for video codecs/bitrates anyway.
    if (!props) {
        std::cerr << "TagLibWrapper: Not an audio file, or no properties found: " << filePath << std::endl;
        // We could create a basic Metadata object, but let's
        // assume for now we only care about full audio files.
        return nullptr;
    }

    // Create a new AudioMetadata object
    auto meta = std::make_unique<AudioMetadata>();

    // --- Fill common Metadata fields ---
    if (tag->title().isEmpty()) {
        // Fallback: use the file name (without extension) as the title
        meta->title = std::filesystem::path(filePath).stem().string();
    } else {
        meta->title = tag->title().toCString(true);
    }
    
    meta->durationInSeconds = props->lengthInSeconds();
    meta->fileSizeInBytes = f.file()->length();

    // --- Fill AudioMetadata specific fields ---
    meta->artist = tag->artist().toCString(true);
    meta->album = tag->album().toCString(true);
    meta->genre = tag->genre().toCString(true);
    meta->year = tag->year();
    
    // We can also add custom fields from TagLib's property map
    TagLib::PropertyMap allTags = f.tag()->properties();
    for(auto const& [key, val] : allTags) {
        //meta->customFields[key] = val.toString().toCString(true);
        meta->customFields[key.toCString(true)] = val.toString().toCString(true);
    }

    return meta; // Return the new object (ownership is moved)
}