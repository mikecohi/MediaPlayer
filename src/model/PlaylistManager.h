#pragma once
#include <vector>
#include <string>
#include <memory>
#include "Playlist.h"

#include "model/MediaManager.h"
/**
 * @class PlaylistManager
 * @brief Manages all Playlist objects (create, delete, find).
 * This class owns all Playlist objects.
 */
class PlaylistManager {
private:
    // The manager owns all Playlist objects via smart pointers
    std::vector<std::unique_ptr<Playlist>> playlists;
    MediaManager* mediaManager;
public:
    explicit PlaylistManager(MediaManager* manager);
    Playlist* createPlaylist(const std::string& name);
    Playlist* getPlaylistByName(const std::string& name);
    bool deletePlaylist(const std::string& name);

    std::vector<Playlist*> getAllPlaylists();

    // Logic for saving/loading playlists to/from disk
    void saveToFile(const std::string& filename);
    void loadFromFile(const std::string& filename);
};