#pragma once
#include <string>
#include <vector>

/**
 * @namespace FileUtils
 * @brief Provides utility functions for file system operations.
 */
namespace FileUtils {
    /**
     * @brief Recursively scans a directory for media files.
     * @param rootPath The absolute or relative path to the root directory.
     * @return A vector of strings, where each string is the full path to a media file.
     */
    std::vector<std::string> getMediaFilesRecursive(const std::string& rootPath);

    /**
     * @brief Checks if a file path has a valid media extension.
     * @param filePath The full path to the file.
     * @return true if it's a recognized media file, false otherwise.
     */
    bool isMediaFile(const std::string& filePath);
} // namespace FileUtils