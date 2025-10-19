#ifndef MEDIAFILE_H
#define MEDIAFILE_H

#include <string>
#include <iostream>
#include <taglib/fileref.h>
#include <taglib/tag.h>

// Represents a single media file (audio/video)
class MediaFile {
private:
    std::string name;   // display name of the file
    std::string path;   // file path on disk (or mock path)
    TagLib::FileRef metadata;

public:
    MediaFile() {}

    // Constructor 
    explicit MediaFile(const std::string& name) : name(name), path("") {}
    MediaFile(const std::string& name, const std::string& path) : name(name), path(path) {}
    MediaFile(const std::string& path);

    // Getters
    const std::string& getFileName() const { return name; }
    const std::string& getFilePath() const { return path; }
    
    

    // Setters
    void setName(const std::string& n) { name = n; }
    void setPath(const std::string& p) { path = p; }

    // For debugging
    void print() const {
        std::cout << "MediaFile: " << name << " (" << path << ")" << std::endl;
    }

    std::string getTitle() const;
    std::string getArtist() const;
    std::string getAlbum() const;
    std::string getGenre() const;
    int getDuration() const;
    unsigned int getYear() const;
    unsigned int getTrack() const;
    bool setMetadata(const std::string& key, const std::string& value);
    bool isValid() const;
    std::string getCustomMetadataField(const std::string& key) const;
};

#endif
