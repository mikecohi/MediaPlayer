#include "model/Playlist.h"
#include <algorithm> // For std::find, std::remove

Playlist::Playlist(const std::string& name) : name(name) {}

std::string Playlist::getName() const {
    return this->name;
}

void Playlist::setName(const std::string& newName) {
    this->name = newName;
}

void Playlist::addTrack(MediaFile* file) {
    if (file == nullptr) {
        return; // Don't add null pointers
    }
    // Optional: Check if track already exists to prevent duplicates
    auto it = std::find(tracks.begin(), tracks.end(), file);
    if (it == tracks.end()) { // Add only if not already present
        tracks.push_back(file);
    }
}

bool Playlist::removeTrack(MediaFile* file) {
    if (file == nullptr) {
        return false;
    }
    // Use std::remove to shift the element to the end (if found)
    auto it = std::remove(tracks.begin(), tracks.end(), file);

    // Check if the element was actually found and shifted
    if (it != tracks.end()) {
        tracks.erase(it, tracks.end()); // Erase the shifted element(s)
        return true;
    }
    return false; // Element not found
}

const std::vector<MediaFile*>& Playlist::getTracks() const {
    return this->tracks;
}