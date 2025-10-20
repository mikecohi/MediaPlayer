#include "model/Playlist.h"
#include <algorithm> // For std::find, std::remove

Playlist::Playlist(const std::string& name) : name(name) {
    // Constructor
}

std::string Playlist::getName() const {
    return this->name;
}

void Playlist::setName(const std::string& newName) {
    this->name = newName;
}

void Playlist::addTrack(MediaFile* file) {
    if (file == nullptr) {
        return;
    }
    // Optional: Check if track already exists
    auto it = std::find(tracks.begin(), tracks.end(), file);
    if (it == tracks.end()) {
        tracks.push_back(file);
    }
}

bool Playlist::removeTrack(MediaFile* file) {
    if (file == nullptr) {
        return false;
    }
    auto it = std::find(tracks.begin(), tracks.end(), file);
    if (it != tracks.end()) {
        tracks.erase(it);
        return true;
    }
    return false;
}

const std::vector<MediaFile*>& Playlist::getTracks() const {
    return this->tracks;
}