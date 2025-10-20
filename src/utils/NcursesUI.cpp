#include "NcursesUI.h"

NcursesUI::NcursesUI() : isInitialized(false), screenHeight(0), screenWidth(0) {
    // Constructor
}

NcursesUI::~NcursesUI() {
    if (isInitialized) {
        closeScreen();
    }
}

bool NcursesUI::initScreen() {
    if (isInitialized) {
        return true; // Already initialized
    }

    // Initialize the main screen
    if (initscr() == NULL) {
        return false; // Ncurses failed to initialize
    }

    // Set up core modes
    cbreak();             // Disable line buffering
    noecho();             // Don't echo keypresses to the screen
    keypad(stdscr, TRUE); // Enable function keys (F1, arrows, etc.)
    curs_set(0);          // Hide the cursor

    // Enable colors
    if (has_colors()) {
        start_color();
        // Initialize default color pair (white on black)
        init_pair(1, COLOR_WHITE, COLOR_BLACK);
        wbkgd(stdscr, COLOR_PAIR(1));
    }

    // Enable mouse support
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);

    // Get screen dimensions
    getScreenDimensions(screenHeight, screenWidth);
    
    isInitialized = true;
    return true;
}

void NcursesUI::closeScreen() {
    if (isInitialized) {
        endwin(); // Restore terminal settings
        isInitialized = false;
    }
}

WINDOW* NcursesUI::drawWindow(int y, int x, int height, int width, const std::string& title) {
    // Create the window
    WINDOW* win = newwin(height, width, y, x);
    
    // Draw a box border
    box(win, 0, 0);

    // Print the title on the top border if provided
    if (!title.empty()) {
        mvwprintw(win, 0, (width - title.length()) / 2, "%s", title.c_str());
    }

    // Refresh the window to show the border and title
    wrefresh(win);
    
    return win;
}

void NcursesUI::printText(WINDOW* win, int y, int x, const std::string& text) {
    if (win == NULL) {
        win = stdscr; // Default to stdscr if window is null
    }
    mvwprintw(win, y, x, "%s", text.c_str());
    // Note: You might need to call wrefresh(win) manually after printing
}

InputEvent NcursesUI::getInput() {
    InputEvent event;
    int ch = wgetch(stdscr); // This is a blocking call

    if (ch == KEY_MOUSE) {
        // This is a mouse event
        MEVENT mouseEvent;
        if (getmouse(&mouseEvent) == OK) {
            event.type = InputEvent::MOUSE;
            event.mouseX = mouseEvent.x;
            event.mouseY = mouseEvent.y;
        }
    } else if (ch != ERR) {
        // This is a keyboard event
        event.type = InputEvent::KEYBOARD;
        event.key = ch;
    }

    return event;
}

void NcursesUI::refreshScreen() {
    refresh();
}

void NcursesUI::clearScreen() {
    clear();
}

void NcursesUI::getScreenDimensions(int& outHeight, int& outWidth) {
    getmaxyx(stdscr, outHeight, outWidth);
    this->screenHeight = outHeight;
    this->screenWidth = outWidth;
}