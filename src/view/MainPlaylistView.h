#pragma once
#include "view/IMainAreaView.h"
#include <vector>
#include <string>
#include "model/PlaylistManager.h"
#include "model/MediaFile.h"

class MainPlaylistView : public IMainAreaView {
public:
    MainPlaylistView(NcursesUI* ui, WINDOW* win, PlaylistManager* manager);
    void draw(FocusArea focus) override;
    void handleInput(InputEvent event, FocusArea focus) override;
    void handleMouse(int localY, int localX) override;
    MediaFile* getSelectedTrack() const;

private:
    NcursesUI* ui;
    WINDOW* win;
    PlaylistManager* playlistManager;
    int playlistSelected;
    int trackSelected;
};