#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <string>
#include <vector>
#include <iostream>
#include "MediaFile.h"

// Represents a single playlist that stores multiple media files
class Playlist {
private:
    std::string name;
    std::vector<MediaFile*> tracks;

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
};

#endif
