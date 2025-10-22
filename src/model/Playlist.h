#pragma once
#include <string>
#include <vector>
#include "MediaFile.h" // Need MediaFile definition

/**
 * @class Playlist
 * @brief Represents a single playlist.
 * This class does NOT own MediaFile objects; it only holds non-owning
 * pointers to files managed by MediaManager.
 */
class Playlist {
public:
    Playlist(const std::string& name);

    std::string getName() const;
    void setName(const std::string& newName);

    /**
     * @brief Adds a track to the playlist.
     * @param file A non-owning pointer to the MediaFile.
     */
    void addTrack(MediaFile* file);

    /**
     * @brief Removes a track from the playlist.
     * @param file The pointer to the MediaFile to remove.
     * @return true if removal was successful, false otherwise.
     */
    bool removeTrack(MediaFile* file);

    /**
     * @brief Gets all tracks in the playlist.
     * @return A const reference to the vector of non-owning pointers.
     */
    const std::vector<MediaFile*>& getTracks() const;

private:
    std::string name;
    // Vector of non-owning (raw) pointers to MediaFile objects
    std::vector<MediaFile*> tracks;
};