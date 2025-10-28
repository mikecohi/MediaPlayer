#include "view/MainFileView.h"
#include "model/MediaFile.h"
#include "model/Metadata.h"
#include <cmath>
#include <algorithm>
#include <tuple>
#include <iostream> // For debug if needed
#include <filesystem>
#include <iomanip>

namespace fs = std::filesystem;

MainFileView::MainFileView(NcursesUI* ui, WINDOW* win, MediaManager* manager)
    : ui(ui), win(win), mediaManager(manager),
      filePage(1), fileSelected(0)
{
    // Initialize button positions (can be done here or in draw)
    editButtonY = editButtonX = editButtonW = 0;
    addButtonY = addButtonX = addButtonW = 0;

    // Calculate pagination based on initial window size
    int width, height;
    getmaxyx(win, height, width);
    itemsPerPage = height - 6; // Reserve lines for header, borders, footer spacing
    if (itemsPerPage < 1) itemsPerPage = 1;

    // Get initial totalPages from manager
    totalPages = mediaManager ? mediaManager->getTotalPages(itemsPerPage) : 1;
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
    mvwprintw(win, height - 1, 2, "DEBUG: Files=%d Pg=%d Sel=%d",
              totalFilesDebug, filePage, fileSelected);
    // --- END DEBUG LOG ---

    // Recalculate totalPages (window might have resized)
    itemsPerPage = height - 6; // Update itemsPerPage based on current height
    if (itemsPerPage < 1) itemsPerPage = 1;
    totalPages = mediaManager ? mediaManager->getTotalPages(itemsPerPage) : 1;
    if (totalPages == 0) totalPages = 1;
    if (filePage > totalPages) filePage = totalPages; // Clamp page number

    // --- Draw list panel ---
    mvwprintw(win, 2, 3, "File List (Page %d/%d)", filePage, totalPages);
    std::vector<MediaFile*> filesOnPage = mediaManager ? mediaManager->getPage(filePage, itemsPerPage) : std::vector<MediaFile*>();

    if (filesOnPage.empty() && totalFilesDebug > 0) {
         mvwprintw(win, 4, 3, "(No files on this page)");
    }

    for (size_t i = 0; i < filesOnPage.size(); ++i) {
        // Ensure we don't draw outside the window height
        if (4 + (int)i >= height - 2) break; // Check against available lines

        int fileIdxGlobal = (filePage - 1) * itemsPerPage + i;
        if (focus == FocusArea::MAIN_LIST && fileIdxGlobal == fileSelected) {
            wattron(win, A_REVERSE | A_BOLD);
        }
        // Truncate filename display
        mvwprintw(win, 4 + i, 3, "%.*s", listWidth - 5, filesOnPage[i]->getFileName().c_str());
        wattroff(win, A_REVERSE | A_BOLD);
    }
     // Optional: Draw Prev/Next page indicators/buttons
     mvwprintw(win, height - 3, 3, "[< Prev]"); // Placeholder
     mvwprintw(win, height - 3, 12, "[Next >]"); // Placeholder


    // --- Draw detail panel ---
    mvwprintw(win, 2, listWidth + 2, "Metadata");

    MediaFile* selectedFile = getSelectedFile();
    if (selectedFile && selectedFile->getMetadata()) {
         mvwprintw(win, 4, listWidth + 2, "Title: %.*s", width-listWidth-4, selectedFile->getMetadata()->title.c_str());
         // (Draw other metadata fields here)
         // Example:
         // mvwprintw(win, 5, listWidth + 2, "Artist: %.*s", width-listWidth-4, selectedFile->getMetadata()->getField("artist").c_str());
         // mvwprintw(win, 6, listWidth + 2, "Album: %.*s", width-listWidth-4, selectedFile->getMetadata()->getField("album").c_str());

    // Metadata content area border
    mvwhline(win, 3, listWidth + 1, ACS_HLINE, detailWidth - 2); // Top border
    mvwhline(win, height - 4, listWidth + 1, ACS_HLINE, detailWidth - 2); // Bottom border
    mvwvline(win, 4, listWidth, ACS_VLINE, height - 7); // Left border (relative to mainWin)
    mvwvline(win, 4, width - 2, ACS_VLINE, height - 7); // Right border
    // Corner pieces
    mvwaddch(win, 3, listWidth, ACS_ULCORNER);
    mvwaddch(win, 3, width - 2, ACS_URCORNER);
    mvwaddch(win, height - 4, listWidth, ACS_LLCORNER);
    mvwaddch(win, height - 4, width - 2, ACS_LRCORNER);


    // Metadata content - ONLY if explicitly selected
    if (fileExplicitlySelected) {
        MediaFile* selectedFile = getSelectedFile();
        if (selectedFile && selectedFile->getMetadata()) {
            Metadata* meta = selectedFile->getMetadata();
            int y = 4;
           // Title
            mvwprintw(win, y++, listWidth + 2, "Title: %.*s", detailWidth-4, meta->title.c_str());
            // Artist
            mvwprintw(win, y++, listWidth + 2, "Artist: %.*s", detailWidth-4, meta->getField("artist").c_str()); 
            // Album
            mvwprintw(win, y++, listWidth + 2, "Album: %.*s", detailWidth-4, meta->getField("album").c_str());
            
            int duration = meta->durationInSeconds;
            char timeStr[12];
            sprintf(timeStr, "%02d:%02d", duration / 60, duration % 60);
            mvwprintw(win, y++, listWidth + 2, "Length: %s", timeStr);

            // 2. Bitrate
            std::string bitrate = meta->getField("bitrate");
            if (bitrate.empty() || bitrate == "0") bitrate = "?";
            mvwprintw(win, y++, listWidth + 2, "Bitrate: %s kbps", bitrate.c_str());

            // 3. Size
            double sizeMB = (double)meta->fileSizeInBytes / (1024.0 * 1024.0);
            char sizeStr[15];
            sprintf(sizeStr, "%.2f MB", sizeMB);
            mvwprintw(win, y++, listWidth + 2, "Size: %s", sizeStr);

            // 4. Item Type (File Extension)
            std::string ext = fs::path(selectedFile->getFilePath()).extension().string();
            mvwprintw(win, y++, listWidth + 2, "Type: %s", ext.c_str());
            
            // 5. Genre
            mvwprintw(win, y++, listWidth + 2, "Genre: %.*s", detailWidth-4, meta->getField("genre").c_str());
        } else {
             mvwprintw(win, 4, listWidth + 2, "(No metadata found)");
        }
    } else {
         mvwprintw(win, 4, listWidth + 2, "Title: N/A");
         if (fileSelected >= 0 && fileSelected < totalFilesDebug) {
             mvwprintw(win, 5, listWidth + 2, "(No metadata loaded?)");
         }
    }

    // Calculate and draw buttons
    editButtonY = height - 4; // Position near bottom
    editButtonX = listWidth + 2;
    std::string editLabel = "[Edit Metadata]";
    editButtonW = editLabel.length();

    addButtonY = height - 3; // Position near bottom
    addButtonX = listWidth + 2;
    std::string addLabel = "[Add to Playlist]";
    addButtonW = addLabel.length();

    // Draw buttons, highlight if detail panel has focus
    if (focus == FocusArea::MAIN_DETAIL) wattron(win, A_REVERSE | A_BOLD);
    mvwprintw(win, editButtonY, editButtonX, "%s", editLabel.c_str());
    mvwprintw(win, addButtonY, addButtonX, "%s", addLabel.c_str());
    wattroff(win, A_REVERSE | A_BOLD);
    // --- END DRAW BUTTONS ---

    wnoutrefresh(win);
}

// Update return type to MainAreaAction
MainAreaAction MainFileView::handleInput(InputEvent event, FocusArea focus) {
    if (!mediaManager) return MainAreaAction::NONE; // Safety check

    int totalFiles = mediaManager->getTotalFileCount();
    if (focus == FocusArea::MAIN_LIST && totalFiles > 0) {
        int oldSelected = fileSelected;
        int oldPage = filePage;

        if (event.key == KEY_DOWN) fileSelected = std::min(fileSelected + 1, totalFiles - 1);
        if (event.key == KEY_UP) fileSelected = std::max(0, fileSelected - 1);
        else if (event.key == KEY_PPAGE) { // Page Up
             filePage = std::max(1, filePage - 1);
             fileSelected = (filePage - 1) * itemsPerPage; // Select top item
        }
        else if (event.key == KEY_NPAGE) { // Page Down
             filePage = std::min(totalPages, filePage + 1);
             fileSelected = (filePage - 1) * itemsPerPage; // Select top item
        }

        // Auto-scroll page if selection moved via UP/DOWN keys
        if (event.key == KEY_UP || event.key == KEY_DOWN) {
             if (fileSelected != oldSelected) {
                 int newPage = (fileSelected / itemsPerPage) + 1;
                 if (newPage != filePage) filePage = newPage;
             }
        }
    }
    else if (focus == FocusArea::MAIN_DETAIL && event.key == 10) { // Enter on detail panel
        // Placeholder: Assume Enter hits "Add to Playlist"
        return MainAreaAction::ADD_TRACK_TO_PLAYLIST;
        // (Later, add logic to select between Edit/Add buttons using Up/Down)
    }
    return MainAreaAction::NONE;
}

// Update return type to MainAreaAction
MainAreaAction MainFileView::handleMouse(int localY, int localX) {
    if (!mediaManager) return MainAreaAction::NONE;

    int listStartY = 4;
    int clickedIndexOnPage = localY - listStartY;
    int width; getmaxyx(win, std::ignore, width); int listWidth = width / 2;

    if (localX < listWidth) { // Click on list panel
        if (clickedIndexOnPage >= 0 && clickedIndexOnPage < itemsPerPage) {
            int clickedIndexGlobal = (filePage - 1) * itemsPerPage + clickedIndexOnPage;
            // Check if clicked index is valid within the total number of files
            if (clickedIndexGlobal < mediaManager->getTotalFileCount()) {
                fileSelected = clickedIndexGlobal;
                // Optional: Trigger play on double-click? (Would need event state)
            }
        }
        // Check clicks on Prev/Next buttons
        int height; getmaxyx(win, height, width); // Get height again
        if (localY == height - 3) { // Clicked on button row
            if (localX >= 3 && localX < 3 + 8) { // Clicked [< Prev]
                filePage = std::max(1, filePage - 1);
                fileSelected = (filePage - 1) * itemsPerPage;
            } else if (localX >= 12 && localX < 12 + 8) { // Clicked [Next >]
                filePage = std::min(totalPages, filePage + 1);
                fileSelected = (filePage - 1) * itemsPerPage;
            }
        }

    } else { // Click on detail panel
        // Check if Edit button clicked (Using stored positions from draw())
        if (localY == editButtonY && localX >= editButtonX && localX < editButtonX + editButtonW) {
             return MainAreaAction::EDIT_METADATA;
        }
        // Check if Add button clicked
        if (localY == addButtonY && localX >= addButtonX && localX < addButtonX + addButtonW) {
             return MainAreaAction::ADD_TRACK_TO_PLAYLIST;
        }
    }
    return MainAreaAction::NONE; // No specific action triggered
}

MediaFile* MainFileView::getSelectedFile() const {
    if (!mediaManager) return nullptr;
    int totalFiles = mediaManager->getTotalFileCount();
    if (fileSelected < 0 || fileSelected >= totalFiles) return nullptr;

    int targetPage = (fileSelected / itemsPerPage) + 1;
    int indexOnPage = fileSelected % itemsPerPage;

    // Make sure calculated page is valid (can happen if itemsPerPage changes)
    if (targetPage < 1 || targetPage > totalPages) {
         std::cerr << "Error in getSelectedFile: Calculated targetPage " << targetPage << " is out of bounds (1-" << totalPages << ")" << std::endl;
         return nullptr;
    }


    std::vector<MediaFile*> pageData = mediaManager->getPage(targetPage, itemsPerPage);
    if (indexOnPage >= 0 && static_cast<size_t>(indexOnPage) < pageData.size()) {
        return pageData[indexOnPage];
    }

    // Log if calculation seems off
    std::cerr << "Error in getSelectedFile: indexOnPage=" << indexOnPage
              << ", pageData.size=" << pageData.size()
              << ", fileSelected=" << fileSelected
              << ", itemsPerPage=" << itemsPerPage
              << ", targetPage=" << targetPage << std::endl;
    return nullptr;
}