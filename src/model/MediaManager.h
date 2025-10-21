#pragma once
#include <vector>
#include <string>
#include <memory>
#include "MediaFile.h"

class TagLibWrapper;

class MediaManager {
public:
    /**
     * @brief Constructor using Dependency Injection.
     * @param tagUtil A non-owning pointer to a TagLibWrapper instance.
     */
    MediaManager(TagLibWrapper* tagUtil);

    /**
     * @brief Scans a directory recursively and loads all media files.
     * @param path The root directory path to scan.
     */
    void loadFromDirectory(const std::string& path);
    void clearLibrary();

    // --- Interface for View (Pagination) ---
    std::vector<MediaFile*> getPage(int pageNumber, int pageSize = 25);
    int getTotalPages(int pageSize = 25) const;
    int getTotalFileCount() const;

private:
    // The library owns all MediaFile objects via smart pointers
    std::vector<std::unique_ptr<MediaFile>> library;
    
    // Non-owning pointer to TagLib utility
    TagLibWrapper* tagUtil;
};