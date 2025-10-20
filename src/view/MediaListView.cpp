#include "view/MediaListView.h"
#include <string>  // For std::to_string
#include <cmath>   // For std::ceil
#include <algorithm> // For std::max/min

MediaListView::MediaListView(NcursesUI* ui)
    : ui(ui), win(nullptr), selectedIndex(0), currentPage(1) 
{
    // Create a full-screen window (1-char margin)
    int screenHeight, screenWidth;
    ui->getScreenDimensions(screenHeight, screenWidth);
    win = ui->drawWindow(0, 0, screenHeight, screenWidth, "Media Library");

    // Calculate itemsPerPage based on window height
    // (height - 2 for borders, - 2 for header/footer)
    itemsPerPage = screenHeight - 4; 
    if (itemsPerPage < 1) itemsPerPage = 1;

    // --- POPULATE MOCK DATA ---
    for (int i = 1; i <= 100; ++i) {
        mockMediaList.push_back("Mock Song " + std::to_string(i) + ".mp3");
    }

    // Calculate total pages
    if (mockMediaList.empty()) {
        totalPages = 1;
    } else {
        totalPages = static_cast<int>(std::ceil(
            static_cast<double>(mockMediaList.size()) / itemsPerPage
        ));
    }
}

MediaListView::~MediaListView() {
    if (win) {
        delwin(win);
    }
}

void MediaListView::draw() {
    wclear(win);
    box(win, 0, 0); 

    // 1. Draw Header
    std::string title = "Media Library";
    mvwprintw(win, 0, (getmaxx(win) - title.length()) / 2, "%s", title.c_str());
    
    std::string pageInfo = "Page " + std::to_string(currentPage) + "/" + std::to_string(totalPages);
    mvwprintw(win, 1, 2, "%s", pageInfo.c_str());

    // 2. Draw List Items
    int start = (currentPage - 1) * itemsPerPage;
    int end = std::min(start + itemsPerPage, static_cast<int>(mockMediaList.size()));

    for (int i = start; i < end; ++i) {
        int displayLine = (i - start) + 2; // +2 to account for header
        std::string text = mockMediaList[i];
        
        // Truncate text if it's too long
        int maxWidth = getmaxx(win) - 4; // -4 for padding
        if (text.length() > static_cast<size_t>(maxWidth)) {
            text = text.substr(0, maxWidth - 3) + "...";
        }

        if (i == selectedIndex) {
            wattron(win, A_REVERSE);
            ui->printText(win, displayLine, 2, text);
            wattroff(win, A_REVERSE);
        } else {
            ui->printText(win, displayLine, 2, text);
        }
    }
    
    // 3. Draw Footer
    std::string footer = "'b' Back | 'q' Quit | Arrows/PgUp/PgDn Navigate";
    mvwprintw(win, getmaxy(win) - 2, 2, "%s", footer.c_str());

    wrefresh(win);
}

void MediaListView::updatePagination() {
    // Check if selectedIndex has moved off the current page
    int currentTopIndex = (currentPage - 1) * itemsPerPage;
    int currentBottomIndex = currentTopIndex + itemsPerPage - 1;

    if (selectedIndex < currentTopIndex) {
        // Scrolled up past the top
        currentPage = (selectedIndex / itemsPerPage) + 1;
    } else if (selectedIndex > currentBottomIndex) {
        // Scrolled down past the bottom
        currentPage = (selectedIndex / itemsPerPage) + 1;
    }
}

ViewId MediaListView::handleInput(InputEvent event) {
    if (event.type != InputEvent::KEYBOARD) {
        return ViewId::STAY;
    }

    int listSize = mockMediaList.size();
    if (listSize == 0) { // Nothing to navigate
        if (event.key == 'b' || event.key == 'q') return ViewId::MAIN_MENU;
        return ViewId::STAY;
    }

    switch (event.key) {
        case KEY_UP:
            selectedIndex = std::max(0, selectedIndex - 1);
            break;

        case KEY_DOWN:
            selectedIndex = std::min(listSize - 1, selectedIndex + 1);
            break;
            
        case KEY_PPAGE: // Page Up
        case KEY_LEFT:
            if (currentPage > 1) {
                currentPage--;
                selectedIndex = (currentPage - 1) * itemsPerPage;
            }
            break;
            
        case KEY_NPAGE: // Page Down
        case KEY_RIGHT:
            if (currentPage < totalPages) {
                currentPage++;
                selectedIndex = (currentPage - 1) * itemsPerPage;
            }
            break;

        case 'b':
            return ViewId::MAIN_MENU;
        case 'q':
            return ViewId::QUIT;
    }
    
    // Update currentPage based on new selectedIndex
    updatePagination();
    
    // Redraw to show changes
    draw();

    return ViewId::STAY;
}