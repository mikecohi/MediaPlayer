#include "view/PopupView.h"
#include <string.h> // For strlen, strncpy

PopupView::PopupView(NcursesUI* ui, int parentHeight, int parentWidth)
    : ui(ui), win(nullptr), parentH(parentHeight), parentW(parentWidth)
{
    // Calculate centered popup dimensions (adjust as needed)
    winHeight = std::min(10, parentH - 4);
    winWidth = std::min(50, parentW - 10);
}

PopupView::~PopupView() {
    clearWindow(); // Ensure window is destroyed
}

void PopupView::drawWindow(const std::string& title) {
    if (win) { // Destroy old window if exists
        delwin(win);
    }
    int y = (parentH - winHeight) / 2;
    int x = (parentW - winWidth) / 2;
    win = ui->drawWindow(y, x, winHeight, winWidth, title);
    keypad(win, TRUE); // Enable arrow keys for the popup window
}

void PopupView::clearWindow() {
    if (win) {
        delwin(win);
        win = nullptr;
        // Important: Need to refresh the underlying screen after closing popup
        // This should be handled by the caller (UIManager) by forcing a redraw.
    }
}

std::optional<std::string> PopupView::showTextInput(const std::string& prompt, const std::string& initialValue) {
    drawWindow("Input Required");
    curs_set(1); // Show cursor
    echo();      // Echo input characters

    mvwprintw(win, 1, 2, "%.*s", winWidth - 4, prompt.c_str());

    // Input field area
    WINDOW* inputWin = derwin(win, 1, winWidth - 4, 3, 2); // Create subwindow for input
    wattron(inputWin, A_REVERSE);
    mvwprintw(inputWin, 0, 0, "%-*s", winWidth - 4, ""); // Clear input area
    wattroff(inputWin, A_REVERSE);

    // Prepare buffer
    char buffer[256]; // Max input length
    strncpy(buffer, initialValue.c_str(), sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0'; // Null-terminate

    wmove(inputWin, 0, 0); // Move cursor to start
    wprintw(inputWin, "%s", buffer); // Print initial value
    wrefresh(inputWin);

    // Get input using wgetnstr (handles basic editing)
    int result = wgetnstr(inputWin, buffer, sizeof(buffer) - 1);

    // Cleanup
    delwin(inputWin);
    noecho();
    curs_set(0);
    clearWindow();

    if (result == ERR || result == KEY_CANCEL || (result == OK && strlen(buffer) == 0 && initialValue.empty()) ) { // Treat empty input as cancel if initial was empty
         return std::nullopt; // Cancelled (e.g., ESC often maps weirdly, ERR can mean cancel)
    } else {
        return std::string(buffer);
    }
}


std::optional<int> PopupView::showListSelection(const std::string& title, const std::vector<std::string>& options) {
    if (options.empty()) return std::nullopt;

    drawWindow(title);
    int selected = 0;
    int scrollOffset = 0;
    int maxDisplay = winHeight - 4; // Lines available for options

    while (true) {
        werase(win);
        box(win, 0, 0);
        mvwprintw(win, 0, (winWidth - title.length()) / 2, "%.*s", winWidth - 2, title.c_str());
        mvwprintw(win, 1, 2, "Use UP/DOWN/ENTER. ESC to cancel.");

        // Draw visible options
        for (int i = 0; i < maxDisplay; ++i) {
            int currentOptionIndex = scrollOffset + i;
            if (currentOptionIndex >= (int)options.size()) break;

            if (currentOptionIndex == selected) {
                wattron(win, A_REVERSE);
            }
            mvwprintw(win, i + 2, 2, "%.*s", winWidth - 4, options[currentOptionIndex].c_str());
            wattroff(win, A_REVERSE);
        }
        wrefresh(win);

        // Get input
        int ch = wgetch(win);
        switch (ch) {
            case KEY_UP:
                selected = std::max(0, selected - 1);
                if (selected < scrollOffset) scrollOffset = selected;
                break;
            case KEY_DOWN:
                selected = std::min((int)options.size() - 1, selected + 1);
                if (selected >= scrollOffset + maxDisplay) scrollOffset = selected - maxDisplay + 1;
                break;
            case 10: // Enter
                clearWindow();
                return selected;
            case 27: // ESC key (may vary)
            case 'q': // Also treat 'q' as cancel
                clearWindow();
                return std::nullopt;
        }
    }
}