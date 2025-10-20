#include "view/MainMenuView.h"

MainMenuView::MainMenuView(NcursesUI* ui) 
    : ui(ui), win(nullptr), selectedOption(0) 
{
    // Define menu options
    options.push_back("Browse Media");
    options.push_back("Playlists");
    options.push_back("Quit");

    // Create a centered window for the menu
    int screenHeight, screenWidth;
    ui->getScreenDimensions(screenHeight, screenWidth);
    
    int winHeight = 10;
    int winWidth = 40;
    int y = (screenHeight - winHeight) / 2;
    int x = (screenWidth - winWidth) / 2;

    win = ui->drawWindow(y, x, winHeight, winWidth, "Main Menu");
}

MainMenuView::~MainMenuView() {
    // Clean up the ncurses window
    if (win) {
        delwin(win);
    }
}

void MainMenuView::draw() {
    // Clear the window interior
    wclear(win);
    // Redraw the border and title
    box(win, 0, 0); 
    mvwprintw(win, 0, (40 - 9) / 2, "Main Menu");
    
    // Print the menu
    printMenu();

    // Refresh the window to show changes
    wrefresh(win);
}

void MainMenuView::printMenu() {
    int y = 3; // Starting line for menu items
    for (size_t i = 0; i < options.size(); ++i) {
        if (static_cast<int>(i) == selectedOption) {
            // Highlight the selected option
            wattron(win, A_REVERSE);
            ui->printText(win, y + i, 4, "> " + options[i]);
            wattroff(win, A_REVERSE);
        } else {
            ui->printText(win, y + i, 4, "  " + options[i]);
        }
    }
}

ViewId MainMenuView::handleInput(InputEvent event) {
    if (event.type != InputEvent::KEYBOARD) {
        return ViewId::STAY; // We only care about keyboard input
    }

    switch (event.key) {
        case KEY_UP:
            selectedOption = (selectedOption - 1 + options.size()) % options.size();
            break;
        
        case KEY_DOWN:
            selectedOption = (selectedOption + 1) % options.size();
            break;
            
        case 10: // 10 is the Enter key
            if (selectedOption == 0) { // Browse Media
                return ViewId::MEDIA_LIST;
            } else if (selectedOption == 1) { // Playlists
                return ViewId::PLAYLIST_MENU;
            } else if (selectedOption == 2) { // Quit
                return ViewId::QUIT;
            }
            break;

        case 'q':
            return ViewId::QUIT;
    }

    // Redraw the view to show the new selection
    draw();
    
    return ViewId::STAY;
}