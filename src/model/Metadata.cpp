#include "model/Metadata.h"

void Metadata::editField(const std::string& key, const std::string& value) {
    // This map-based approach handles custom keys automatically.
    // Specific logic (like updating 'title' member) can be added.
    if (key == "title") {
        this->title = value;
    }
    // Store it in the map regardless
    this->customFields[key] = value;
}

std::string Metadata::getField(const std::string& key) const {
    if (key == "title") {
        return this->title;
    }
    
    auto it = this->customFields.find(key);
    if (it != this->customFields.end()) {
        return it->second;
    }
    
    return ""; // Return empty string if not found
}