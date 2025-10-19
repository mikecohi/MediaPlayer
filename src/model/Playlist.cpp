#include "Playlist.h"

Playlist::Playlist(const std::string& name) : name(name) {}

void Playlist::addTrack(const MediaFile& file) {
    tracks.push_back(file);
}

void Playlist::removeTrack(const std::string& fileName) {
    for (std::vector<MediaFile>::iterator it = tracks.begin(); it != tracks.end(); ++it) {
        if (it->getFileName() == fileName) {
            tracks.erase(it);
            break;
        }
    }
}

const std::vector<MediaFile>& Playlist::getTracks() const {
    return tracks;
}

const std::string& Playlist::getName() const {
    return name;
}

void Playlist::printToTerminal() const {
    std::cout << "Playlist: " << name << std::endl;
    if (tracks.empty()) {
        std::cout << "  (No tracks)" << std::endl;
        return;
    }

    for (size_t i = 0; i < tracks.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << tracks[i].getFileName() << std::endl;
    }
}
