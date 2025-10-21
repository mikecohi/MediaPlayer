#include "view/MainFileView.h"
#include "model/MediaFile.h"
#include "model/Metadata.h"
#include <cmath>
#include <algorithm>
#include <tuple>
#include <iostream> // For debug if needed

MainFileView::MainFileView(NcursesUI* ui, WINDOW* win, MediaManager* manager)
    : ui(ui), win(win), mediaManager(manager),
      filePage(1), fileSelected(0)
{
    int width, height;
    getmaxyx(win, height, width);

    itemsPerPage = height - 6;
    if (itemsPerPage < 1) itemsPerPage = 1;

    totalPages = mediaManager->getTotalPages(itemsPerPage);
    if (totalPages == 0) totalPages = 1;
}

void MainFileView::draw(FocusArea focus) {
    werase(win);
    box(win, 0, 0);

    int width, height;
    getmaxyx(win, height, width);
    int listWidth = width / 2;

    // --- DEBUG LOG ---
    int totalFilesDebug = 0;
    if (mediaManager) totalFilesDebug = mediaManager->getTotalFileCount();
    mvwprintw(win, height - 1, 2, "DEBUG: Draw | Files=%d Page=%d Sel=%d",
              totalFilesDebug, filePage, fileSelected);
    // --- END DEBUG LOG ---

    totalPages = mediaManager->getTotalPages(itemsPerPage);
    if (totalPages == 0) totalPages = 1;
    if (filePage > totalPages) filePage = totalPages;

    // Draw list panel
    mvwprintw(win, 2, 3, "File List (Page %d/%d)", filePage, totalPages);
    std::vector<MediaFile*> filesOnPage = mediaManager->getPage(filePage, itemsPerPage);

    if (filesOnPage.empty() && totalFilesDebug > 0) {
         mvwprintw(win, 4, 3, "Error: No files on this page?");
    }

    for (size_t i = 0; i < filesOnPage.size(); ++i) {
        int fileIdxGlobal = (filePage - 1) * itemsPerPage + i;
        if (focus == FocusArea::MAIN_LIST && fileIdxGlobal == fileSelected) {
            wattron(win, A_REVERSE | A_BOLD);
        }
        mvwprintw(win, 4 + i, 3, "%.*s", listWidth - 5, filesOnPage[i]->getFileName().c_str());
        wattroff(win, A_REVERSE | A_BOLD);
    }

    // Draw detail panel
    mvwprintw(win, 2, listWidth + 2, "Metadata");
    if (focus == FocusArea::MAIN_DETAIL) wattron(win, A_REVERSE | A_BOLD);
    mvwprintw(win, 8, listWidth + 2, "[Edit Metadata]");
    wattroff(win, A_REVERSE | A_BOLD);

    MediaFile* selectedFile = getSelectedFile();
    if (selectedFile && selectedFile->getMetadata()) {
         mvwprintw(win, 4, listWidth + 2, "Title: %s", selectedFile->getMetadata()->title.c_str());
    } else {
         mvwprintw(win, 4, listWidth + 2, "Title: N/A");
         if (fileSelected >= 0 && fileSelected < totalFilesDebug) {
             mvwprintw(win, 5, listWidth + 2, "(getSelectedFile failed?)");
         }
    }

    wnoutrefresh(win);
}

void MainFileView::handleInput(InputEvent event, FocusArea focus) {
    int totalFiles = mediaManager->getTotalFileCount();
    if (focus == FocusArea::MAIN_LIST && totalFiles > 0) {
        int oldSelected = fileSelected;
        if (event.key == KEY_DOWN) fileSelected = std::min(fileSelected + 1, totalFiles - 1);
        if (event.key == KEY_UP) fileSelected = std::max(0, fileSelected - 1);
        // Page Up/Down Keys
        else if (event.key == KEY_PPAGE && filePage > 1) {
             filePage--;
             fileSelected = (filePage - 1) * itemsPerPage; // Select top item
        }
        else if (event.key == KEY_NPAGE && filePage < totalPages) {
             filePage++;
             fileSelected = (filePage - 1) * itemsPerPage; // Select top item
        }

        // Auto-scroll page if selection changes
        if (fileSelected != oldSelected) {
            int newPage = (fileSelected / itemsPerPage) + 1;
            if (newPage != filePage) filePage = newPage;
        }
    }
    // TODO: Handle input for MAIN_DETAIL (e.g., Enter on [Edit Metadata])
}

void MainFileView::handleMouse(int localY, int localX) {
    int listStartY = 4; // Files start at line Y=4
    int clickedIndexOnPage = localY - listStartY;

    // Check if click is within the list area (left panel)
    int width;
    getmaxyx(win, std::ignore, width);
    if (localX < width / 2) {
        if (clickedIndexOnPage >= 0 && clickedIndexOnPage < itemsPerPage) {
            int clickedIndexGlobal = (filePage - 1) * itemsPerPage + clickedIndexOnPage;
            if (clickedIndexGlobal < mediaManager->getTotalFileCount()) {
                fileSelected = clickedIndexGlobal;
            }
        }
    }
    // TODO: Handle clicks in the detail area (right panel)
}

MediaFile* MainFileView::getSelectedFile() const {
    int totalFiles = mediaManager->getTotalFileCount();
    if (fileSelected < 0 || fileSelected >= totalFiles) return nullptr;
    int targetPage = (fileSelected / itemsPerPage) + 1;
    int indexOnPage = fileSelected % itemsPerPage;
    std::vector<MediaFile*> pageData = mediaManager->getPage(targetPage, itemsPerPage);
    if (indexOnPage >= 0 && static_cast<size_t>(indexOnPage) < pageData.size()) {
        return pageData[indexOnPage];
    }
    std::cerr << "Error in getSelectedFile: indexOnPage=" << indexOnPage
              << ", pageData.size=" << pageData.size() << std::endl;
    return nullptr;
}