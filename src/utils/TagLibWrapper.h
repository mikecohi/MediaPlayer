#pragma once
#include <string>
#include <memory>
#include "model/Metadata.h" // Include our abstract base class

/**
 * @class TagLibWrapper
 * @brief A wrapper class for the TagLib library.
 * Its responsibility is to read metadata from a file and
 * create our application's own Metadata objects.
 */
class TagLibWrapper {
public:
    TagLibWrapper();
    ~TagLibWrapper();

    /**
     * @brief Reads all supported tags from a given media file.
     * @param filePath The full path to the media file.
     * @return A std::unique_ptr to a new Metadata object (or a subclass
     * like AudioMetadata). Returns nullptr if the file cannot be
     * read or is not a supported audio file.
     */
    std::unique_ptr<Metadata> readTags(const std::string& filePath);
    bool writeTags(const std::string& filePath, Metadata* metadata);
};