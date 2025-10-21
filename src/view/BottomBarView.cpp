#include "view/BottomBarView.h"
#include "model/MediaFile.h"
#include <string>

BottomBarView::BottomBarView(NcursesUI* ui, MediaPlayer* player, WINDOW* win)
    : ui(ui), player(player), win(win) 
{}

void BottomBarView::draw(bool hasFocus) {
    wclear(win);
    box(win, 0, 0); // Vẽ viền
    
    std::string title = "Stopped";
    int currentTime = 0;
    int totalTime = 0;
    
    // --- LẤY DỮ LIỆU ---
    if (player != nullptr && player->getState() != PlayerState::STOPPED && player->getCurrentTrack()) {
        // Giai đoạn 3: Sẽ lấy dữ liệu thật
        title = player->getCurrentTrack()->getFileName();
        currentTime = player->getCurrentTime();
        totalTime = player->getTotalTime();
    } else {
        // Giai đoạn 2: Dùng mock data
        title = "Mock Song Title.mp3";
        currentTime = 45;
        totalTime = 200; // 3:20
    }
    
    // Format
    // Sửa lỗi Warning: Tăng kích thước buffer lên 30
    char timeStr[30]; 
    sprintf(timeStr, "%02d:%02d / %02d:%02d", 
            currentTime / 60, currentTime % 60, 
            totalTime / 60, totalTime % 60);

    // --- VẼ ---
    mvwprintw(win, 1, 2, "Now Playing: %s", title.c_str());
    mvwprintw(win, 1, getmaxx(win) - 22, "%s", timeStr);

    // ===================================
    // SỬA LỖI HIỂN THỊ Ở ĐÂY
    // Thay thế ký tự Unicode bằng ASCII
    // ===================================
    
    // Thay vì "───────────────────●───────────────────────────"
    mvwprintw(win, 2, 2, "-------------------o---------------------------");
    
    // Thay vì "<<   ||   >>" (dùng 'II' thay cho '||')
    mvwprintw(win, 3, 2, "<<   II   >>     Vol-   Vol+");
    
    if (hasFocus) mvwprintw(win, 1, 1, ">"); // Dấu hiệu focus
    
    wrefresh(win);
}