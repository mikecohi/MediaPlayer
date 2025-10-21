#include "view/BottomBarView.h"
#include "model/MediaFile.h"
#include <string>
#include <vector>
#include <algorithm> // for std::max/min

BottomBarView::BottomBarView(NcursesUI* ui, MediaPlayer* player, WINDOW* win)
    : ui(ui), player(player), win(win)
{}

void BottomBarView::draw(bool hasFocus) {
    werase(win);
    box(win, 0, 0);

    std::string title = "Stopped";
    int currentTime = 0;
    int totalTime = 0;
    PlayerState state = PlayerState::STOPPED;

    if (player != nullptr) {
        state = player->getState();
        if (state != PlayerState::STOPPED && player->getCurrentTrack()) {
            title = player->getCurrentTrack()->getFileName();
            currentTime = player->getCurrentTime();
            totalTime = player->getTotalTime();
        }
    }

    // Ensure totalTime is not zero to avoid division by zero
    if (totalTime <= 0) totalTime = 1;

    char timeStr[30];
    sprintf(timeStr, "%02d:%02d / %02d:%02d",
            currentTime / 60, currentTime % 60,
            totalTime / 60, totalTime % 60);

    int barWidth = getmaxx(win) - 4; // Max width for progress bar
    int progressChars = 0;
    if (totalTime > 0 && currentTime >= 0) {
        progressChars = static_cast<int>(
            (static_cast<double>(currentTime) / totalTime) * barWidth
        );
        progressChars = std::max(0, std::min(progressChars, barWidth -1));
    }

    std::string progressBar(barWidth, '-');
    if (barWidth > 0 && progressChars < barWidth) {
         progressBar[progressChars] = 'o';
    }

    // Draw Line 1: Title and Time
    mvwprintw(win, 1, 2, "Now Playing: %.*s", getmaxx(win) - 25, title.c_str());
    mvwprintw(win, 1, getmaxx(win) - strlen(timeStr) - 2, "%s", timeStr);

    // Draw Line 2: Progress Bar
    mvwprintw(win, 2, 2, "%s", progressBar.c_str());

    // Draw Line 3: Controls
    const char* playPauseIcon = (state == PlayerState::PLAYING) ? "II" : "> ";
    mvwprintw(win, 3, 2, "<<   %s   >>     Vol-   Vol+", playPauseIcon);

    if (hasFocus) mvwprintw(win, 1, 1, ">");

    wnoutrefresh(win);
}

bool BottomBarView::handleMouse(int localY, int localX) {
    if (localY != 3) return false; // Only handle clicks on the control line

    // Approximate X coordinates of buttons
    int prevX = 2; int prevW = 2;
    int playPauseX = prevX + prevW + 3; int playPauseW = 2; // "II" or "> "
    int nextX = playPauseX + playPauseW + 3; int nextW = 2;
    // int volDownX = nextX + nextW + 5; int volDownW = 4;
    // int volUpX = volDownX + volDownW + 3; int volUpW = 4;

    if (localX >= playPauseX && localX < playPauseX + playPauseW) {
        return true; // Signal UIManager to toggle play/pause
    }
    // TODO: Add checks for prevX, nextX, etc., and return a different value or enum
    //       to tell UIManager *which* button was clicked.
    //       For now, only Play/Pause returns true.

    return false;
}