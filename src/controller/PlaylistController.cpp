#include "PlaylistController.h"
#include <iostream>

PlaylistController::PlaylistController(PlaylistManager* pm, MediaController* mc)
    : playlistManager(pm), mediaController(mc), view(NULL) {}

void PlaylistController::setView(PlaylistMenuView* v) {
    view = v;
}

void PlaylistController::addTrackToPlaylist(const std::string& playlistName, const MediaFile& file) {
    Playlist* p = playlistManager->findPlaylist(playlistName);
    if (p) {
        p->addTrack(file);
        std::cout << "Added track to playlist: " << playlistName << std::endl;
    }
}

void PlaylistController::removeTrackFromPlaylist(const std::string& playlistName, const std::string& fileName) {
    Playlist* p = playlistManager->findPlaylist(playlistName);
    if (p) {
        p->removeTrack(fileName);
        std::cout << "Removed track from playlist: " << fileName << std::endl;
    }
}

void PlaylistController::playPlaylist(const std::string& playlistName) {
    Playlist* p = playlistManager->findPlaylist(playlistName);
    if (!p) {
        std::cout << "Playlist not found: " << playlistName << std::endl;
        return;
    }

    const std::vector<MediaFile>& tracks = p->getTracks();
    for (size_t i = 0; i < tracks.size(); ++i) {
        mediaController->play(tracks[i]);  // use MediaController to play
    }
}

void PlaylistController::showMenu() {
    if (!view) {
        view = new PlaylistMenuView(playlistManager); // fallback if not injected
        view->setController(this);
    }

    view->displayMenu();
}

void PlaylistController::handleUserChoice(int choice) {
    switch (choice) {
        case 1: {
            std::string playlistName, fileName;
            std::cout << "Enter playlist name: ";
            std::getline(std::cin, playlistName);
            std::cout << "Enter file name to add: ";
            std::getline(std::cin, fileName);
            MediaFile mf(fileName); // dummy MediaFile
            addTrackToPlaylist(playlistName, mf);
            break;
        }
        case 2: {
            std::string playlistName, fileName;
            std::cout << "Enter playlist name: ";
            std::getline(std::cin, playlistName);
            std::cout << "Enter file name to remove: ";
            std::getline(std::cin, fileName);
            removeTrackFromPlaylist(playlistName, fileName);
            break;
        }
        case 3: {
            std::string playlistName;
            std::cout << "Enter playlist name to play: ";
            std::getline(std::cin, playlistName);
            playPlaylist(playlistName);
            break;
        }
        default:
            std::cout << "Invalid choice." << std::endl;
            break;
    }

    // After handling, re-display the menu
    view->displayMenu();
}
