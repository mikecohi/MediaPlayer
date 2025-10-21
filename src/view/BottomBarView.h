#pragma once
#include "utils/NcursesUI.h"
#include "model/MediaPlayer.h"

class BottomBarView {
public:
    BottomBarView(NcursesUI* ui, MediaPlayer* player, WINDOW* win);
    void draw(bool hasFocus);
    bool handleMouse(int localY, int localX);

private:
    NcursesUI* ui;
    MediaPlayer* player;
    WINDOW* win;
};