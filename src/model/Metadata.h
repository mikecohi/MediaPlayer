#pragma once
#include <string>
#include <map>

/**
 * @class Metadata
 * @brief Base class for storing media file metadata.
 * Uses a map to support dynamic key-value pairs as required.
 */
class Metadata {
public:
    virtual ~Metadata() = default; // Virtual destructor for base class

    // Common fields for all media types
    std::string title;
    int durationInSeconds = 0;
    long fileSizeInBytes = 0;

    // Use a map to support "add new keys" requirement
    std::map<std::string, std::string> customFields;

    /**
     * @brief Edits or adds a metadata field.
     * @param key The metadata key (e.g., "artist", "year").
     * @param value The new value for the key.
     */
    virtual void editField(const std::string& key, const std::string& value);

    /**
     * @brief Retrieves a metadata field.
     * @param key The key to retrieve.
     * @return The value as a string.
     */
    virtual std::string getField(const std::string& key) const;
};