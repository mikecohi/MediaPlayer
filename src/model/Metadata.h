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
    Metadata();
    virtual ~Metadata() = default; // Virtual destructor for base class

    // Common fields for all media types
    std::string title;
    int durationInSeconds = 0;
    long fileSizeInBytes = 0;

    // Use a map to support "add new keys" requirement
    //std::map<std::string, std::string> customFields;
    //virtual void editField(const std::string& key, const std::string& value);
    
    virtual std::string getField(const std::string& key) const;
    void setField(const std::string& key, const std::string& value);
protected:
    std::map<std::string, std::string> fields;
};