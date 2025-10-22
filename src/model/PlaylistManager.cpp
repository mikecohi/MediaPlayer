#include "model/PlaylistManager.h"
#include "model/MediaManager.h"
#include <algorithm> // For std::find_if and std::remove_if
#include <iostream>
#include <fstream>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

//Constructor 
PlaylistManager::PlaylistManager(MediaManager* manager) : mediaManager(manager) {
    if (mediaManager == nullptr) {
         std::cerr << "CRITICAL: PlaylistManager initialized with null MediaManager!" << std::endl;
        // Consider throwing an exception or setting an error state
    }
}

Playlist* PlaylistManager::createPlaylist(const std::string& name) {
    // 1. Check if a playlist with this name already exists
    if (getPlaylistByName(name) != nullptr) {
        std::cerr << "PlaylistManager: Playlist with name '" << name << "' already exists." << std::endl;
        return nullptr;
    }

    // 2. Create the new playlist
    auto newPlaylist = std::make_unique<Playlist>(name);
    
    // 3. Get the raw pointer to return
    Playlist* ptr = newPlaylist.get();
    
    // 4. Add the new playlist (and its ownership) to the vector
    this->playlists.push_back(std::move(newPlaylist));
    
    return ptr;
}

bool PlaylistManager::deletePlaylist(const std::string& name) {
    // Use the erase-remove idiom to find and delete the playlist
    auto it = std::remove_if(playlists.begin(), playlists.end(), 
        [&name](const std::unique_ptr<Playlist>& p) {
            return p->getName() == name;
        });

    if (it != playlists.end()) {
        // Erase the unique_ptr, which triggers the Playlist's destructor
        playlists.erase(it, playlists.end());
        return true;
    }

    // Not found
    return false;
}

Playlist* PlaylistManager::getPlaylistByName(const std::string& name) {
    // Find the first playlist that matches the name
    auto it = std::find_if(playlists.begin(), playlists.end(),
        [&name](const std::unique_ptr<Playlist>& p) {
            return p->getName() == name;
        });

    if (it != playlists.end()) {
        return it->get(); // Return the raw pointer
    }

    return nullptr; // Not found
}

std::vector<Playlist*> PlaylistManager::getAllPlaylists() {
    std::vector<Playlist*> ptrs;
    ptrs.reserve(this->playlists.size());

    // Fill the vector with non-owning pointers
    for (const auto& p : this->playlists) {
        ptrs.push_back(p.get());
    }
    
    return ptrs;
}

// --- IMPLEMENT saveToFile ---
void PlaylistManager::saveToFile(const std::string& filename) {
    json jsonData = json::array(); // Root is a JSON array

    for (const auto& playlistPtr : playlists) {
        if (!playlistPtr) continue; // Safety check

        json playlistObj; // JSON object for this playlist
        playlistObj["name"] = playlistPtr->getName();

        json tracksArray = json::array(); // JSON array for track paths
        for (const MediaFile* trackPtr : playlistPtr->getTracks()) {
            if (trackPtr) { // Ensure track pointer is valid
                tracksArray.push_back(trackPtr->getFilePath()); // Store the full path
            }
        }
        playlistObj["tracks"] = tracksArray;

        jsonData.push_back(playlistObj); // Add playlist object to root array
    }

    // Write JSON data to the file
    std::ofstream outFile(filename);
    if (outFile.is_open()) {
        try {
            // dump(4) for pretty printing with 4 spaces indent
            outFile << jsonData.dump(4);
            outFile.close();
            std::cout << "PlaylistManager: Playlists saved to " << filename << std::endl;
        } catch (const json::exception& e) {
             std::cerr << "PlaylistManager Error: Failed to serialize JSON for saving: " << e.what() << std::endl;
             outFile.close(); // Close file even on error
        }
    } else {
        std::cerr << "PlaylistManager Error: Could not open file for saving: " << filename << std::endl;
    }
}

// --- IMPLEMENT loadFromFile ---
void PlaylistManager::loadFromFile(const std::string& filename) {
    if (mediaManager == nullptr) {
        std::cerr << "PlaylistManager Error: Cannot load playlists - MediaManager is null." << std::endl;
        return;
    }

    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        // This is not necessarily an error, maybe the file doesn't exist yet
        std::cout << "PlaylistManager Info: Playlist file not found or could not be opened: " << filename << ". Starting fresh." << std::endl;
        return;
    }

    try {
        json jsonData = json::parse(inFile);
        inFile.close(); // Close file after parsing

        // Clear existing playlists before loading
        playlists.clear();

        if (!jsonData.is_array()) {
            std::cerr << "PlaylistManager Error: Invalid playlist file format in " << filename << ". Expected a JSON array." << std::endl;
            return;
        }

        // Iterate through each playlist object in the JSON array
        for (const auto& playlistObj : jsonData) {
            // Basic validation of the playlist object structure
            if (!playlistObj.is_object() || !playlistObj.contains("name") || !playlistObj["name"].is_string() || !playlistObj.contains("tracks") || !playlistObj["tracks"].is_array()) {
                 std::cerr << "PlaylistManager Warning: Skipping invalid or incomplete playlist object in " << filename << "." << std::endl;
                 continue;
            }

            std::string name = playlistObj["name"];
            // Use createPlaylist to add the new playlist (handles duplicates if needed, though loading should clear first)
            Playlist* newPlaylist = createPlaylist(name);

            if (newPlaylist) {
                // Iterate through the array of track paths in the JSON
                for (const auto& trackPathJson : playlistObj["tracks"]) {
                    if (trackPathJson.is_string()) {
                        std::string trackPath = trackPathJson;
                        // IMPORTANT: Find the corresponding MediaFile pointer in MediaManager
                        MediaFile* trackPtr = mediaManager->findFileByPath(trackPath);
                        if (trackPtr) {
                            newPlaylist->addTrack(trackPtr); // Add the valid pointer
                        } else {
                            // Log missing tracks but continue loading
                            std::cerr << "PlaylistManager Warning: Track path from playlist '" << name
                                      << "' not found in current MediaManager library, skipping: " << trackPath << std::endl;
                        }
                    } else {
                         std::cerr << "PlaylistManager Warning: Invalid track path type in playlist '" << name << "', skipping." << std::endl;
                    }
                }
            } else {
                 std::cerr << "PlaylistManager Warning: Could not create playlist '" << name << "' during load (maybe duplicate name?)." << std::endl;
            }
        }
        std::cout << "PlaylistManager: Playlists loaded successfully from " << filename << std::endl;

    } catch (json::parse_error& e) {
        std::cerr << "PlaylistManager Error: Failed to parse playlist JSON file " << filename << ": " << e.what() << std::endl;
        if(inFile.is_open()) inFile.close(); // Ensure file is closed on error
    } catch (const std::exception& e) {
         std::cerr << "PlaylistManager Error: An unexpected error occurred during playlist loading: " << e.what() << std::endl;
         if(inFile.is_open()) inFile.close();
    }
}