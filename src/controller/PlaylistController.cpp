#include "controller/PlaylistController.h"
#include <iostream> 

// Constructor
PlaylistController::PlaylistController(PlaylistManager* manager)
    : playlistManager(manager) 
{
    if (this->playlistManager == nullptr) {
        std::cerr << "CRITICAL: PlaylistController initialized with a null PlaylistManager!" << std::endl;
    }
}

// createPlaylist 
bool PlaylistController::createPlaylist(const std::string& name) {
    if (name.empty()) {
        std::cerr << "Controller Error: Playlist name cannot be empty." << std::endl;
        return false;
    }
    Playlist* p = playlistManager->createPlaylist(name);
    //return (p != nullptr);
    if (p) {
        playlistManager->autoSave(); // <-- LƯU NGAY
        return true;
    }
    return false;
}

// deletePlaylist
bool PlaylistController::deletePlaylist(const std::string& name) {
    if (name.empty()) {
        return false;
    }
    //return playlistManager->deletePlaylist(name);
    bool success = playlistManager->deletePlaylist(name);
    if (success) {
        playlistManager->autoSave(); // <-- LƯU NGAY
    }
    return success;
}

// addTrackToPlaylist 
bool PlaylistController::addTrackToPlaylist(MediaFile* file, Playlist* playlist) {
    if (file == nullptr || playlist == nullptr) {
        std::cerr << "Controller Error: Cannot add null track or to null playlist." << std::endl;
        return false;
    }
    
    // We delegate the logic to the Model (Playlist object)
    playlist->addTrack(file); 
    
    // Here we could add logic to save the playlist to file
    playlistManager->autoSave();
    
    return true;
}

// removeTrackFromPlaylist - PHẢI KHỚP VỚI FILE .h
bool PlaylistController::removeTrackFromPlaylist(MediaFile* file, Playlist* playlist) {
    if (file == nullptr || playlist == nullptr) {
        std::cerr << "Controller Error: Cannot remove null track or from null playlist." << std::endl;
        return false;
    }
    
    bool success = playlist->removeTrack(file);
    if (success) {
        playlistManager->autoSave(); // <-- LƯU NGAY
    }
    return success;
}

