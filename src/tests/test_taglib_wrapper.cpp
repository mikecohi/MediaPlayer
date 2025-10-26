#include "utils/TagLibWrapper.h" 
#include "model/AudioMetadata.h" 
#include <iostream>

int main() {
    TagLibWrapper taglib;
    std::string path = "/home/quynhmai/mock/MediaPlayer/test_media/Dreamers.mp3";
    auto meta = taglib.readTags(path);

    if (meta) {
        std::cout << "Title: " << meta->title << "\n";
        std::cout << "Artist: " << static_cast<AudioMetadata*>(meta.get())->artist << "\n";
    } else {
        std::cout << "Failed to read tags.\n";
    }

    return 0;
}
