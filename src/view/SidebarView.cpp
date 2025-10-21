#include "view/SidebarView.h"
#include "view/UIManager.h" // Cần AppMode

SidebarView::SidebarView(NcursesUI* ui, WINDOW* win)
    : ui(ui), win(win), selectedOption(0), exitFlag(false)
{
    options = {"FILE", "PLAYLIST", "USB", "EXIT"};
}

void SidebarView::draw(bool hasFocus) {
    wclear(win);
    box(win, 0, 0);
    mvwprintw(win, 0, 5, "Navigate");
    
    for (size_t i = 0; i < options.size(); ++i) {
        if (static_cast<int>(i) == selectedOption) {
            wattron(win, A_REVERSE);
            if (hasFocus) wattron(win, A_BOLD); // Highlight thêm nếu đang focus
        }
        
        mvwprintw(win, 3 + i, 3, "%s", options[i].c_str());
        wattroff(win, A_REVERSE | A_BOLD);
    }
    wrefresh(win);
}

AppMode SidebarView::handleInput(InputEvent event) {
    if (event.type != InputEvent::KEYBOARD) return AppMode::FILE_BROWSER; // Giá trị rỗng

    switch(event.key) {
        case KEY_UP:
            selectedOption = (selectedOption - 1 + options.size()) % options.size();
            break;
        case KEY_DOWN:
            selectedOption = (selectedOption + 1) % options.size();
            break;
        case 10: // Enter
            if (selectedOption == 0) return AppMode::FILE_BROWSER;
            if (selectedOption == 1) return AppMode::PLAYLISTS;
            if (selectedOption == 2) return AppMode::FILE_BROWSER; // Tái sử dụng FILE
            if (selectedOption == 3) exitFlag = true;
            break;
    }
    // Trả về chế độ hiện tại nếu không thay đổi
    return (selectedOption == 1) ? AppMode::PLAYLISTS : AppMode::FILE_BROWSER;
}

bool SidebarView::shouldExit() const {
    return exitFlag;
}