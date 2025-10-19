#ifndef PLAYLIST_CONTROLLER_H
#define PLAYLIST_CONTROLLER_H

#include "../model/PlaylistManager.h"
#include "MediaController.h"
#include "../view/PlaylistMenuView.h"

// Controller: handles user actions related to playlists
class PlaylistController {
private:
    PlaylistManager* playlistManager;
    MediaController* mediaController;
    PlaylistMenuView* view; // View pointer for MVC coordination

public:
    PlaylistController(PlaylistManager* pm, MediaController* mc);

    // Connect a view (dependency injection)
    void setView(PlaylistMenuView* v);

    // Core actions
    void addTrackToPlaylist(const std::string& playlistName, const MediaFile& file);
    void removeTrackFromPlaylist(const std::string& playlistName, const std::string& fileName);
    void playPlaylist(const std::string& playlistName);

    // Display menu (controller entry)
    void showMenu();

    // Called by view when user selects menu options
    void handleUserChoice(int choice);
};

#endif
