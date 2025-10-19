#include "PlaylistManager.h"
#include "MediaFile.h"


Playlist* PlaylistManager::createPlaylist(const std::string& name) {
    playlists.push_back(Playlist(name));
    return &playlists.back(); // return pointer point to recent added playlist
}

void PlaylistManager::deletePlaylist(const std::string& name) {
    for (std::vector<Playlist>::iterator it = playlists.begin(); it != playlists.end(); ++it) {
        if (it->getName() == name) {
            playlists.erase(it);
            break;
        }
    }
}

Playlist* PlaylistManager::findPlaylist(const std::string& name) {
    for (size_t i = 0; i < playlists.size(); ++i) {
        if (playlists[i].getName() == name) {
            return &playlists[i];
        }
    }
    return 0; // not found
}

const std::vector<Playlist>& PlaylistManager::getAllPlaylists() const {
    return playlists;
}

void PlaylistManager::loadMockData() {
    createPlaylist("Favorites");
    createPlaylist("Chill Vibes");
    createPlaylist("Workout Mix");

    MediaFile song1("song1.mp3");
    MediaFile song2("song2.mp3");
    MediaFile song3("video1.mp4");

    Playlist* fav = findPlaylist("Favorites");
    if (fav) {
        fav->addTrack(song1);
        fav->addTrack(song3);
    }

    Playlist* chill = findPlaylist("Chill Vibes");
    if (chill) {
        chill->addTrack(song2);
    }
}
