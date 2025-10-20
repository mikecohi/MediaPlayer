#pragma once
#include <vector>
#include <string>
#include <memory>
#include "Playlist.h"

/**
 * @class PlaylistManager
 * @brief Manages all Playlist objects (create, delete, find).
 * This class owns all Playlist objects.
 */
class PlaylistManager {
public:
    /**
     * @brief Creates a new, empty playlist.
     * @param name The name for the new playlist.
     * @return A non-owning pointer to the created Playlist, or nullptr if name exists.
     */
    Playlist* createPlaylist(const std::string& name);

    /**
     * @brief Deletes a playlist by name.
     * @param name The name of the playlist to delete.
     * @return true if deletion was successful, false otherwise.
     */
    bool deletePlaylist(const std::string& name);

    /**
     * @brief Finds a playlist by its name.
     * @param name The name to search for.
     * @return A non-owning pointer to the Playlist, or nullptr if not found.
     */
    Playlist* getPlaylistByName(const std::string& name);

    /**
     * @brief Gets all created playlists.
     * @return A vector of non-owning pointers to all Playlists.
     */
    std::vector<Playlist*> getAllPlaylists();

    // Logic for saving/loading playlists to/from disk
    void saveToFile(const std::string& filename);
    void loadFromFile(const std::string& filename);

private:
    // The manager owns all Playlist objects via smart pointers
    std::vector<std::unique_ptr<Playlist>> playlists;
};