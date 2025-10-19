#ifndef PLAYLIST_MANAGER_H
#define PLAYLIST_MANAGER_H

#include <vector>
#include <string>
#include "Playlist.h"

// Manages multiple playlists, allows create/delete/find operations
class PlaylistManager {
private:
    std::vector<Playlist> playlists;

public:
    Playlist* createPlaylist(const std::string& name);
    void deletePlaylist(const std::string& name);
    Playlist* findPlaylist(const std::string& name);
    const std::vector<Playlist>& getAllPlaylists() const;

    void loadMockData(); // Load mock playlists for testing
};

#endif
