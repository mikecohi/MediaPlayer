#include "model/PlaylistManager.h"
#include "model/MediaManager.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include "nlohmann/json.hpp"
#include <filesystem>
using json = nlohmann::json;
namespace fs = std::filesystem;

// Constructor
PlaylistManager::PlaylistManager(MediaManager* manager) : mediaManager(manager) {
    if (mediaManager == nullptr) {
        std::cerr << "CRITICAL: PlaylistManager initialized with null MediaManager!" << std::endl;
    }
}

PlaylistManager::~PlaylistManager() {
    std::cout << "[DEBUG] PlaylistManager Destructor: Clearing all playlists..." << std::endl;
    for (const auto& p : playlists) {
        if (p) {
            std::cout << "[DEBUG]   Deleting playlist object: " << p->getName() << std::endl;
        }
    }
    playlists.clear();
    std::cout << "[DEBUG] PlaylistManager Destructor done." << std::endl;
}

Playlist* PlaylistManager::createPlaylist(const std::string& name) {
    if (getPlaylistByName(name) != nullptr) {
        std::cerr << "PlaylistManager: Playlist with name '" << name << "' already exists." << std::endl;
        return nullptr;
    }

    auto newPlaylist = std::make_unique<Playlist>(name);
    Playlist* ptr = newPlaylist.get();
    playlists.push_back(std::move(newPlaylist));

    std::cout << "[DEBUG] PlaylistManager: Created playlist '" << name << "' at " << ptr << std::endl;
    return ptr;
}

bool PlaylistManager::deletePlaylist(const std::string& name) {
    auto it = std::remove_if(playlists.begin(), playlists.end(),
        [&name](const std::unique_ptr<Playlist>& p) {
            return p->getName() == name;
        });

    if (it != playlists.end()) {
        for (auto del = it; del != playlists.end(); ++del) {
            if (del->get()) {
                std::cout << "[DEBUG] PlaylistManager: Deleting playlist '" 
                          << del->get()->getName() << "' at " << del->get() << std::endl;
            }
        }
        playlists.erase(it, playlists.end());
        std::cout << "[DEBUG] PlaylistManager: Delete complete." << std::endl;
        return true;
    }

    std::cerr << "[DEBUG] PlaylistManager: Delete failed (playlist not found): " << name << std::endl;
    return false;
}

Playlist* PlaylistManager::getPlaylistByName(const std::string& name) {
    auto it = std::find_if(playlists.begin(), playlists.end(),
        [&name](const std::unique_ptr<Playlist>& p) {
            return p->getName() == name;
        });

    return (it != playlists.end()) ? it->get() : nullptr;
}

std::vector<Playlist*> PlaylistManager::getAllPlaylists() {
    std::vector<Playlist*> ptrs;
    ptrs.reserve(playlists.size());
    for (const auto& p : playlists) {
        ptrs.push_back(p.get());
    }
    return ptrs;
}

// --- SAVE TO FILE ---
void PlaylistManager::saveToFile(const std::string& filename) {
    std::string path_to_save = filename;
    if (path_to_save.empty()) {
        path_to_save = savePath_; // Dùng đường dẫn đã lưu nếu không có path mới
    }

    if (path_to_save.empty()) {
        std::cerr << "PlaylistManager Error: No save path specified. Cannot save." << std::endl;
        return;
    }
    json jsonData = json::array();

    for (const auto& playlistPtr : playlists) {
        if (!playlistPtr) continue;

        json playlistObj;
        playlistObj["name"] = playlistPtr->getName();

        json tracksArray = json::array();
        for (const MediaFile* trackPtr : playlistPtr->getTracks()) {
            if (trackPtr) {
                tracksArray.push_back(trackPtr->getFilePath());
            }
        }
        playlistObj["tracks"] = tracksArray;
        jsonData.push_back(playlistObj);
    }

    // std::ofstream outFile(filename);
    // if (outFile.is_open()) {
    //     try {
    //         outFile << jsonData.dump(4);
    //         outFile.close();
    //         std::cout << "PlaylistManager: Playlists saved to " << filename << std::endl;
    //     } catch (const json::exception& e) {
    //         std::cerr << "PlaylistManager Error: Failed to serialize JSON: " << e.what() << std::endl;
    //     }
    // } else {
    //     std::cerr << "PlaylistManager Error: Could not open file for saving: " << filename << std::endl;
    // }
    try {
        fs::path p(path_to_save);
        if (p.has_parent_path()) {
            fs::create_directories(p.parent_path()); // Tự tạo thư mục
        }

        std::ofstream outFile(path_to_save);
        if (outFile.is_open()) {
            outFile << jsonData.dump(4);
            outFile.close();
            std::cout << "PlaylistManager: Playlists saved to " << path_to_save << std::endl;
        } else {
            std::cerr << "PlaylistManager Error: Could not open file for saving: " << path_to_save << std::endl;
        }
    } catch (fs::filesystem_error& e) {
         std::cerr << "PlaylistManager Error: Filesystem error: " << e.what() << std::endl;
    } catch (const json::exception& e) {
         std::cerr << "PlaylistManager Error: Failed to serialize JSON: " << e.what() << std::endl;
    }
}

// --- LOAD FROM FILE ---
void PlaylistManager::loadFromFile(const std::string& filename) {
    if (mediaManager == nullptr) {
        std::cerr << "PlaylistManager Error: Cannot load playlists - MediaManager is null." << std::endl;
        return;
    }
    savePath_ = filename;
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        std::cout << "PlaylistManager Info: No playlist file found: " << filename << std::endl;
        return;
    }

    try {
        json jsonData = json::parse(inFile);
        inFile.close();

        std::cout << "[DEBUG] PlaylistManager: Clearing existing playlists before loading..." << std::endl;
        playlists.clear();

        if (!jsonData.is_array()) {
            std::cerr << "PlaylistManager Error: Invalid format in " << filename << std::endl;
            return;
        }

        for (const auto& playlistObj : jsonData) {
            if (!playlistObj.is_object()) continue;
            if (!playlistObj.contains("name") || !playlistObj["name"].is_string()) continue;

            std::string name = playlistObj["name"];
            Playlist* newPlaylist = createPlaylist(name);

            if (newPlaylist) {
                for (const auto& trackPathJson : playlistObj["tracks"]) {
                    if (trackPathJson.is_string()) {
                        std::string trackPath = trackPathJson;
                        MediaFile* trackPtr = mediaManager->findFileByPath(trackPath);
                        if (trackPtr) {
                            newPlaylist->addTrack(trackPtr);
                        } else {
                            std::cerr << "PlaylistManager Warning: Track missing: " << trackPath << std::endl;
                        }
                    }
                }
            }
        }

        std::cout << "PlaylistManager: Playlists loaded successfully from " << filename << std::endl;

    } catch (json::parse_error& e) {
        std::cerr << "PlaylistManager Error: Failed to parse " << filename << ": " << e.what() << std::endl;
    }
}

void PlaylistManager::autoSave() {
    std::cout << "[DEBUG] PlaylistManager: Auto-saving changes..." << std::endl;
    saveToFile(savePath_); 
}