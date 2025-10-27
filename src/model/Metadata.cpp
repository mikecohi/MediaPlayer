#include "model/Metadata.h"

Metadata::Metadata() : durationInSeconds(0) {
    // initialize
    fields["artist"] = "";
    fields["album"] = "";
    fields["genre"] = "";
    fields["year"] = "0";
}

// void Metadata::editField(const std::string& key, const std::string& value) {
//     // This map-based approach handles custom keys automatically.
//     // Specific logic (like updating 'title' member) can be added.
//     if (key == "title") {
//         this->title = value;
//     }
//     // Store it in the map regardless
//     this->customFields[key] = value;
// }

std::string Metadata::getField(const std::string& key) const {
    if (key == "title") {
        return this->title;
    }
    
    auto it = this->fields.find(key);
    if (it != this->fields.end()) {
        return it->second;
    }
    
    return ""; // Return empty string if not found
}

void Metadata::setField(const std::string& key, const std::string& value) {
    // 1. Cập nhật các trường public nếu khớp
    if (key == "title") {
        this->title = value;
    }
    
    // else if (key == "artist") {
    //     this->artist = value; 
    // }

    // 2. Luôn cập nhật map nội bộ
    // (Nó sẽ tự động thêm mới nếu key chưa tồn tại, hoặc cập nhật nếu đã có)
    this->fields[key] = value;
}