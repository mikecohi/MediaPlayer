#include "model/MediaManager.h"
#include "utils/TagLibWrapper.h" // Need full definition
#include "utils/FileUtils.h"     // Include for static functions
#include <iostream>
#include <cmath> // For std::ceil

MediaManager::MediaManager(TagLibWrapper* tagUtil)
    : tagUtil(tagUtil) 
{
    // Constructor initializer list
}

void MediaManager::loadFromDirectory(const std::string& path) {
    std::cout << "MediaManager: Loading from directory: " << path << std::endl;
    
    // 1. Clear existing library
    this->clearLibrary();

    // 2. Get all media file paths from FileUtils (static call)
    std::vector<std::string> files = FileUtils::getMediaFilesRecursive(path);
    
    std::cout << "MediaManager: Found " << files.size() << " media files." << std::endl;

    // 3. Loop through files
    for (const auto& file : files) {
        // 4. Use TagLibWrapper to read tags
        std::unique_ptr<Metadata> metadata = this->tagUtil->readTags(file);
        
        // 5. If metadata was read successfully, create MediaFile
        if (metadata) {
            // Create the MediaFile and add it to the library
            this->library.push_back(std::make_unique<MediaFile>(file, std::move(metadata)));
        } else {
            std::cerr << "MediaManager: Skipping file (could not read metadata): " << file << std::endl;
        }
    }
    
    std::cout << "MediaManager: Load complete. Library size: " << this->library.size() << std::endl;
}

void MediaManager::clearLibrary() {
    this->library.clear();
}

std::vector<MediaFile*> MediaManager::getPage(int pageNumber, int pageSize) {
    std::vector<MediaFile*> page;
    
    if (pageNumber < 1) {
        pageNumber = 1;
    }

    int start = (pageNumber - 1) * pageSize;
    
    if (start < 0 || static_cast<size_t>(start) >= this->library.size()) {
        return page; // Return empty vector
    }

    int end = start + pageSize;
    
    if (static_cast<size_t>(end) > this->library.size()) {
        end = this->library.size();
    }

    for (int i = start; i < end; ++i) {
        page.push_back(this->library[i].get()); // Add non-owning pointer
    }
    
    return page;
}

int MediaManager::getTotalPages(int pageSize) const {
    if (pageSize <= 0) return 0;
    if (this->library.empty()) return 1; // Always at least one page, even if empty
    
    return static_cast<int>(std::ceil(static_cast<double>(this->library.size()) / pageSize));
}

int MediaManager::getTotalFileCount() const {
    return this->library.size();
}