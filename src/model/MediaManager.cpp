#include "model/MediaManager.h"
#include "utils/TagLibWrapper.h"
#include "utils/FileUtils.h"
#include <iostream>
#include <cmath>
#include <algorithm>

MediaManager::MediaManager(TagLibWrapper* tagUtil)
    : tagUtil(tagUtil) {}

void MediaManager::loadFromDirectory(const std::string& path) {
    std::cout << "MediaManager: Loading from directory: " << path << std::endl;
    this->clearLibrary();

    std::vector<std::string> files = FileUtils::getMediaFilesRecursive(path);
    std::cout << "MediaManager: Found " << files.size() << " media files." << std::endl;

    for (const auto& file : files) {
        std::unique_ptr<Metadata> metadata = this->tagUtil->readTags(file);
        if (metadata) {
            auto mediaPtr = std::make_unique<MediaFile>(file, std::move(metadata));
            std::cout << "[DEBUG] MediaManager: Adding file " << file << " at " << mediaPtr.get() << std::endl;
            this->library.push_back(std::move(mediaPtr));
        } else {
            std::cerr << "MediaManager: Skipping (no metadata): " << file << std::endl;
        }
    }

    std::cout << "MediaManager: Load complete. Library size: " << this->library.size() << std::endl;
}

void MediaManager::clearLibrary() {
    std::cout << "[DEBUG] MediaManager::clearLibrary(): Deleting " << library.size() << " files..." << std::endl;
    for (const auto& f : library) {
        if (f) {
            std::cout << "   [DEBUG] Deleting MediaFile: " << f->getFilePath() << " at " << f.get() << std::endl;
        }
    }
    
    library.clear();
    std::cout << "[DEBUG] MediaManager::clearLibrary() done." << std::endl;
}

std::vector<MediaFile*> MediaManager::getPage(int pageNumber, int pageSize) {
    std::vector<MediaFile*> page;
    if (pageNumber < 1) pageNumber = 1;

    int start = (pageNumber - 1) * pageSize;
    if (start < 0 || static_cast<size_t>(start) >= library.size()) return page;

    int end = std::min<int>(start + pageSize, library.size());
    for (int i = start; i < end; ++i) {
        page.push_back(library[i].get());
    }
    return page;
}

int MediaManager::getTotalPages(int pageSize) const {
    if (pageSize <= 0) return 0;
    if (library.empty()) return 1;
    return static_cast<int>(std::ceil(static_cast<double>(library.size()) / pageSize));
}

int MediaManager::getTotalFileCount() const {
    return library.size();
}

MediaFile* MediaManager::findFileByPath(const std::string& filePath) const {
    auto it = std::find_if(library.begin(), library.end(),
        [&filePath](const std::unique_ptr<MediaFile>& filePtr) {
            return filePtr->getFilePath() == filePath;
        });
    if (it != library.end()) {
        return it->get();
    }
    return nullptr;
}
