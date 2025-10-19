#ifndef PLAYLIST_DETAIL_VIEW_H
#define PLAYLIST_DETAIL_VIEW_H

#include "../model/Playlist.h"
#include "../controller/PlaylistController.h"
#include <iostream>

// View: hiển thị chi tiết và thao tác với 1 playlist cụ thể
class PlaylistDetailView {
private:
    Playlist* playlist;                // Model (1 playlist)
    PlaylistController* controller;    // Controller liên quan

public:
    // Constructor hợp lệ: có playlist và controller
    PlaylistDetailView(Playlist* p, PlaylistController* c);

    // Hiển thị chi tiết playlist và menu tương tác
    void show();

private:
    // Các hành động trong View (UI-level)
    void displayDetails();
    void handleUserInput();

    // Gọi qua Controller
    void addTrack();
    void removeTrack();
    void play();
};

#endif
