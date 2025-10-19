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
    std::vector<MediaFile> tracks;

public:
    explicit Playlist(const std::string& name);

    void addTrack(const MediaFile& file);
    void removeTrack(const std::string& fileName);
    const std::vector<MediaFile>& getTracks() const;

    const std::string& getName() const;
    void printToTerminal() const; // Display track list on terminal
};

#endif
