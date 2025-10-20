#include "FileUtils.h"
#include <filesystem> // Requires C++17
#include <iostream>   // For error logging
#include <set>
#include <string>
#include <algorithm> // For std::transform

namespace {
    // Helper function to convert a string to lowercase
    std::string toLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        return s;
    }
} // Anonymous namespace

bool FileUtils::isMediaFile(const std::string& filePath) {
    // Use a static set for efficient (O(log N)) lookup
    static const std::set<std::string> mediaExtensions = {
        // Audio Formats
        ".mp3", ".wav", ".flac", ".aac", ".ogg", ".m4a",
        // Video Formats
        ".mp4", ".mkv", ".avi", ".mov", ".wmv", ".flv"
    };

    try {
        std::string ext = std::filesystem::path(filePath).extension().string();
        return mediaExtensions.count(toLower(ext));
    } catch (std::exception& e) {
        std::cerr << "Error checking file extension: " << e.what() << std::endl;
        return false;
    }
}

std::vector<std::string> FileUtils::getMediaFilesRecursive(const std::string& rootPath) {
    std::vector<std::string> mediaFiles;
    
    try {
        // Create a path object
        std::filesystem::path fsRootPath(rootPath);
        
        // Check if the path exists and is a directory
        if (!std::filesystem::exists(fsRootPath) || !std::filesystem::is_directory(fsRootPath)) {
            std::cerr << "Error: Path is not a valid directory: " << rootPath << std::endl;
            return mediaFiles; // Return empty vector
        }

        // Use recursive_directory_iterator to scan all subdirectories
        for (const auto& entry : std::filesystem::recursive_directory_iterator(fsRootPath)) {
            // Check if it's a regular file (not a directory, symlink, etc.)
            if (entry.is_regular_file()) {
                std::string pathString = entry.path().string();
                if (isMediaFile(pathString)) {
                    mediaFiles.push_back(pathString);
                }
            }
        }
    } catch (std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    } catch (std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
    }

    return mediaFiles;
}