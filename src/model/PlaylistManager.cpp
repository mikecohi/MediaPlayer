#include "model/PlaylistManager.h"
#include <algorithm> // For std::find_if and std::remove_if
#include <iostream>

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

void PlaylistManager::saveToFile(const std::string& filename) {
    // (Implementation for saving to JSON, XML, or binary file)
    std::cout << "PlaylistManager: saveToFile() not yet implemented." << std::endl;
}

void PlaylistManager::loadFromFile(const std::string& filename) {
    // (Implementation for loading from JSON, XML, or binary file)
    std::cout << "PlaylistManager: loadFromFile() not yet implemented." << std::endl;
}