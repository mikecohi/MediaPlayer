#ifndef PLAYLIST_MENU_VIEW_H
#define PLAYLIST_MENU_VIEW_H

#include "../model/PlaylistManager.h"
#include <iostream>

class PlaylistController; // forward declare

// View: displays playlists and sends actions back to controller
class PlaylistMenuView {
private:
    PlaylistManager* playlistManager;
    PlaylistController* controller;

public:
    PlaylistMenuView(PlaylistManager* pm);
    void setController(PlaylistController* ctrl);

    void displayMenu();
};

#endif
