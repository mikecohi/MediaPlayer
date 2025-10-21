#pragma once
#include "utils/NcursesUI.h"
#include "model/MediaPlayer.h" // <-- Sẵn sàng cho Giai đoạn 3

class BottomBarView {
public:
    /**
     * @brief Constructor
     * @param player Non-owning pointer. Giai đoạn 2 có thể là NULL.
     */
    BottomBarView(NcursesUI* ui, MediaPlayer* player, WINDOW* win);
    
    /**
     * @brief Draws the view. Sẽ dùng mock data nếu player là NULL.
     */
    void draw(bool hasFocus);

private:
    NcursesUI* ui;
    MediaPlayer* player; // <-- Con trỏ đến Model
    WINDOW* win;
};