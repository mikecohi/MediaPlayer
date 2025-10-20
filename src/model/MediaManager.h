#pragma once
#include <vector>
#include <string>
#include <memory>
#include "MediaFile.h"

// Forward-declare utility classes
// We only need a pointer to TagLibWrapper.
// FileUtils is a namespace, so we just include it in the .cpp
class TagLibWrapper;

/**
 * @class MediaManager
 * @brief Manages the entire media library.
 * This class owns all MediaFile objects.
 */
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

    /**
     * @brief Clears all loaded media files from the library.
     */
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