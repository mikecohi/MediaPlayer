#include "view/MainUSBView.h"
#include "controller/AppController.h"
#include "model/Metadata.h"
#include <cmath>
#include <iostream>
#include <algorithm>

MainUSBView::MainUSBView(NcursesUI* ui, WINDOW* win, AppController* controller)
    : ui(ui), win(win), appController(controller),
      mediaManager(nullptr),
      usbConnected(false),
      fileExplicitlySelected(false),
      filePage(1),
      fileSelected(-1)
{
    prevBtnY = prevBtnX = prevBtnW = 0;
    nextBtnY = nextBtnX = nextBtnW = 0;
    reloadBtnY = reloadBtnX = reloadBtnW = 0;
    ejectBtnY = ejectBtnX = ejectBtnW = 0;
    editBtnY = editBtnX = editBtnW = 0;
    addBtnY = addBtnX = addBtnW = 0;

    updateUSBStatus();

        // Calculate initial pagination (will be recalculated in draw)
    int width, height;
    getmaxyx(win, height, width);
    // Calculate max lines available for files (excluding borders, header, footer/buttons)
    int availableLines = height - 4 - 2; // -4 box, -2 header/footer line
    if(availableLines < 1) availableLines = 1;
    // Limit items per page to 25 or available lines, whichever is smaller
    itemsPerPage = std::min(25, availableLines);

    totalPages = mediaManager ? mediaManager->getTotalPages(itemsPerPage) : 1;
    if (totalPages == 0) totalPages = 1;
}

// --- Kiểm tra trạng thái USB ---
void MainUSBView::updateUSBStatus() {
    if (!appController) return;
    mediaManager = appController->getUSBMediaManager();

    // Kiểm tra USB hiện có (chỉ xem dữ liệu hiện tại, không load lại)
    if (mediaManager && mediaManager->getTotalFileCount() > 0) {
        usbConnected = true;
    } else {
        usbConnected = false;
    }
}


// --- DRAW FUNCTION ---
void MainUSBView::draw(FocusArea focus) {
    werase(win);
    box(win, 0, 0);

    int width, height;
    getmaxyx(win, height, width);
    int listWidth = width / 2;
    int detailWidth = width - listWidth;

    // --- Trường hợp KHÔNG có USB ---
    if (!usbConnected || !mediaManager || mediaManager->getTotalFileCount() == 0) {
        std::string msg = "⚠️ No USB detected. Please connect a USB drive.";
        mvwprintw(win, height / 2, (width - msg.size()) / 2, "%s", msg.c_str());

        // --- Nút Reload và Eject ---
        std::string reloadLabel = "[Reload USB]";
        std::string ejectLabel = "[Eject USB]";
        reloadBtnW = reloadLabel.length();
        ejectBtnW = ejectLabel.length();
        reloadBtnY = height - 3;
        ejectBtnY = height - 3;
        reloadBtnX = (width / 2) - reloadBtnW - 2;
        ejectBtnX = (width / 2) + 2;

        mvwprintw(win, reloadBtnY, reloadBtnX, "%s", reloadLabel.c_str());
        mvwprintw(win, ejectBtnY, ejectBtnX, "%s", ejectLabel.c_str());

        wnoutrefresh(win);
        return;
    }

    // --- Có USB: hiển thị danh sách media ---
    int availableLines = height - 4 - 2; // chừa 3 dòng cuối cho nút
    if(availableLines < 1) availableLines = 1;
    itemsPerPage = std::min(25, availableLines);
    totalPages = mediaManager->getTotalPages(itemsPerPage);
    if (totalPages == 0) totalPages = 1;
    if (filePage > totalPages) filePage = totalPages;

    std::string pageInfo = "Page " + std::to_string(filePage) + "/" + std::to_string(totalPages);
    std::string prevLabel = "[< Prev]";
    std::string nextLabel = "[Next >]";
    prevBtnY = 2; prevBtnX = 3;
    nextBtnY = 2; nextBtnX = listWidth - nextLabel.size() - 2;
    mvwprintw(win, prevBtnY, prevBtnX, "%s", prevLabel.c_str());
    mvwprintw(win, 2, (listWidth - pageInfo.size()) / 2, "%s", pageInfo.c_str());
    mvwprintw(win, nextBtnY, nextBtnX, "%s", nextLabel.c_str());

   // File list content (loop up to itemsPerPage)
    std::vector<MediaFile*> filesOnPage = mediaManager ? mediaManager->getPage(filePage, itemsPerPage) : std::vector<MediaFile*>();
    for (size_t i = 0; i < filesOnPage.size(); ++i) {
        int lineY = 4 + i;
        if (lineY >= height - 2) break; // Don't draw outside window

        // The global index of the current file being drawn
        int fileIdxGlobal = (filePage - 1) * itemsPerPage + i;

        // Highlight if this file is the currently selected one
        if (focus == FocusArea::MAIN_LIST && fileIdxGlobal == fileSelected) {
            wattron(win, A_REVERSE | A_BOLD);
        }

        // Draw the filename from the filesOnPage vector using index 'i'
        mvwprintw(win, lineY, 3, "%.*s", listWidth - 5, filesOnPage[i]->getFileName().c_str());
        
        wattroff(win, A_REVERSE | A_BOLD);
    }

    // --- Metadata khung ---
    std::string metaTitle = "Metadata";
    editBtnY = 2;
    std::string editLabel = "[Edit]";
    editBtnW = editLabel.length();
    int metaTitleX = listWidth + (detailWidth - metaTitle.size()) / 2;
    editBtnX = width - editBtnW - 2;
    mvwprintw(win, 2, metaTitleX, "%s", metaTitle.c_str());
    mvwprintw(win, editBtnY, editBtnX, "%s", editLabel.c_str());

    mvwhline(win, 3, listWidth + 1, ACS_HLINE, detailWidth - 2);
    mvwhline(win, height - 4, listWidth + 1, ACS_HLINE, detailWidth - 2); // Bottom border
    mvwvline(win, 4, listWidth, ACS_VLINE, height - 7);
    mvwvline(win, 4, width - 2, ACS_VLINE, height - 7);

        mvwaddch(win, 3, listWidth, ACS_ULCORNER);
    mvwaddch(win, 3, width - 2, ACS_URCORNER);
    mvwaddch(win, height - 4, listWidth, ACS_LLCORNER);
    mvwaddch(win, height - 4, width - 2, ACS_LRCORNER);

 // Metadata content - ONLY if explicitly selected
    if (fileExplicitlySelected) {
        MediaFile* selectedFile = getSelectedFile();
        if (selectedFile && selectedFile->getMetadata()) {
             mvwprintw(win, 4, listWidth + 2, "Title: %.*s", detailWidth-4, selectedFile->getMetadata()->title.c_str());
             // Draw other fields...
             mvwprintw(win, 5, listWidth + 2, "Artist: %.*s", detailWidth-4, selectedFile->getMetadata()->getField("artist").c_str());
             mvwprintw(win, 6, listWidth + 2, "Album: %.*s", detailWidth-4, selectedFile->getMetadata()->getField("album").c_str());

        } else {
             mvwprintw(win, 4, listWidth + 2, "(No metadata found)");
        }
    } else {
         mvwprintw(win, 4, listWidth + 2, "(Select a file to view details)"); // Initial message
    }

    // --- Buttons: Add to Playlist + Reload + Eject ---
    std::string addLabel = "[Add to Playlist]";
    addBtnW = addLabel.length();
    addBtnY = height - 3;
    addBtnX = listWidth + (detailWidth - addBtnW) / 2;
        // Highlight button if detail panel has focus
    if (focus == FocusArea::MAIN_DETAIL) wattron(win, A_REVERSE | A_BOLD);
    mvwprintw(win, addBtnY, addBtnX, "%s", addLabel.c_str());
    wattroff(win, A_REVERSE | A_BOLD);


    std::string reloadLabel = "[Reload USB]";
    std::string ejectLabel = "[Eject USB]";
    reloadBtnW = reloadLabel.length();
    ejectBtnW = ejectLabel.length();
    reloadBtnY = height - 3;
    ejectBtnY = height - 3;
    reloadBtnX = 3;
    ejectBtnX = (listWidth - ejectBtnW - 2);
    mvwprintw(win, reloadBtnY, reloadBtnX, "%s", reloadLabel.c_str());
    mvwprintw(win, ejectBtnY, ejectBtnX, "%s", ejectLabel.c_str());

    wnoutrefresh(win);
}

// --- INPUT HANDLER ---
MainAreaAction MainUSBView::handleInput(InputEvent event, FocusArea focus) {
    if (!usbConnected || !mediaManager) return MainAreaAction::NONE;

   int totalFiles = mediaManager->getTotalFileCount();
    bool selectionChanged = false; // Flag to check if selection actually moved

    if (focus == FocusArea::MAIN_LIST && totalFiles > 0) {
        //int oldSelected = fileSelected;
        //int oldPage = filePage; // No longer needed here

        if (event.key == KEY_DOWN) {
            fileSelected = std::min(fileSelected + 1, totalFiles - 1);
            selectionChanged = true;
        } else if (event.key == KEY_UP) {
            // Handle initial state where fileSelected might be -1
            if (fileSelected <= 0) fileSelected = 0;
            else fileSelected = std::max(0, fileSelected - 1);
            selectionChanged = true;
        } else if (event.key == KEY_PPAGE) { // Page Up
             if (filePage > 1) {
                 filePage--;
                 fileSelected = (filePage - 1) * itemsPerPage; // Select top item
                 selectionChanged = true;
             }
        } else if (event.key == KEY_NPAGE) { // Page Down
             if (filePage < totalPages) {
                 filePage++;
                 fileSelected = (filePage - 1) * itemsPerPage; // Select top item
                 fileSelected = std::min(fileSelected, totalFiles -1); // Clamp selection
                 selectionChanged = true;
             }
        }

        // Auto-scroll page if selection moved via UP/DOWN
        if (selectionChanged && (event.key == KEY_UP || event.key == KEY_DOWN)) {
             int newPage = (fileSelected < 0) ? 1 : (fileSelected / itemsPerPage) + 1;
             if (newPage != filePage) filePage = newPage;
        }

        // Set flag if user made a valid selection
        if (selectionChanged && fileSelected >= 0) {
            fileExplicitlySelected = true; // Mark as selected
        }


    } else if (focus == FocusArea::MAIN_DETAIL) {
        if (event.key == 10) { // Enter
            // Activate "Add to Playlist" if a file is explicitly selected
            if(fileExplicitlySelected && getSelectedFile() != nullptr) {
                return MainAreaAction::ADD_TRACK_TO_PLAYLIST;
            } else {
                 flash(); // Indicate nothing selected
            }
        }
        // (Add KEY_UP/KEY_DOWN for button selection later)
    }
    return MainAreaAction::NONE;
}

// --- MOUSE HANDLER ---
MainAreaAction MainUSBView::handleMouse(int y, int x) {
    if (!mediaManager) return MainAreaAction::NONE;
    int totalFiles = mediaManager->getTotalFileCount();
    int listStartY = 4;
    int clickedIndexOnPage = y - listStartY;
    int width; getmaxyx(win, std::ignore, width); int listWidth = width / 2;   

    if (y == reloadBtnY && x >= reloadBtnX && x < reloadBtnX + reloadBtnW) {
        appController->loadUSBLibrary();
        updateUSBStatus();
        return MainAreaAction::NONE;
    }
    if (y == ejectBtnY && x >= ejectBtnX && x < ejectBtnX + ejectBtnW) {
        appController->ejectUSB();
        updateUSBStatus();
        return MainAreaAction::NONE;
    }
       // Check Header Button Clicks
    if (y == prevBtnY) { // Clicked on header row
        if (x >= prevBtnX && x < prevBtnX + prevBtnW && filePage > 1) { // Prev
            filePage--;
            fileSelected = (filePage - 1) * itemsPerPage;
            if(fileSelected >= 0) fileExplicitlySelected = true;
            return MainAreaAction::NONE;
        }
        if (x >= nextBtnX && x < nextBtnX + nextBtnW && filePage < totalPages) { // Next
            filePage++;
            fileSelected = (filePage - 1) * itemsPerPage;
            fileSelected = std::min(fileSelected, totalFiles -1);
            if(fileSelected >= 0) fileExplicitlySelected = true;
            return MainAreaAction::NONE;
        }
        if (x >= editBtnX && x < editBtnX + editBtnW) { // Edit
             // Only allow editing if a file has been selected
             if (fileExplicitlySelected && getSelectedFile() != nullptr) {
                 return MainAreaAction::EDIT_METADATA;
             } else {
                 flash();
             }
        }
    }


    // Check List/Detail Panel Clicks
    if (x < listWidth) { // Click on list panel
        if (clickedIndexOnPage >= 0 && clickedIndexOnPage < itemsPerPage) {
            int clickedIndexGlobal = (filePage - 1) * itemsPerPage + clickedIndexOnPage;
            if (clickedIndexGlobal < totalFiles) {
                fileSelected = clickedIndexGlobal;
                fileExplicitlySelected = true; // Mark as selected
            }
        }
    } else { // Click on detail panel
        // Check Add button click
        if (y == addBtnY && x >= addBtnX && x < addBtnX + addBtnW) {
             if (fileExplicitlySelected && getSelectedFile() != nullptr) {
                return MainAreaAction::ADD_TRACK_TO_PLAYLIST;
             } else {
                 flash(); // Indicate no file selected
             }
        }
        // (Could add click handling for metadata area itself)
    }
    return MainAreaAction::NONE;
}






// --- GETSELECTEDFILE FUNCTION (No changes needed from previous version) ---
MediaFile* MainUSBView::getSelectedFile() const {
    if (!mediaManager) return nullptr;
    int totalFiles = mediaManager->getTotalFileCount();
    if (fileSelected < 0 || fileSelected >= totalFiles) return nullptr;

    // Ensure itemsPerPage is valid before division
    if (itemsPerPage <= 0) {
        std::cerr << "Error: itemsPerPage is invalid in getSelectedFile: " << itemsPerPage << std::endl;
        return nullptr;
    }

    int targetPage = (fileSelected / itemsPerPage) + 1;
    int indexOnPage = fileSelected % itemsPerPage;

    // Recalculate totalPages for robust check
    int currentTotalPages = mediaManager->getTotalPages(itemsPerPage);
    if(currentTotalPages == 0) currentTotalPages = 1;

    if (targetPage < 1 || targetPage > currentTotalPages) {
         // This can happen briefly if totalFiles changes and selection wasn't updated
         // std::cerr << "Warning: targetPage " << targetPage << " out of bounds (1-" << currentTotalPages << ")" << std::endl;
         return nullptr; // Return null if page calc is wrong
    }

    std::vector<MediaFile*> pageData = mediaManager->getPage(targetPage, itemsPerPage);
    if (indexOnPage >= 0 && static_cast<size_t>(indexOnPage) < pageData.size()) {
        return pageData[indexOnPage];
    }

    // Only log actual errors, not transient states
    if (static_cast<size_t>(indexOnPage) >= pageData.size()){
      std::cerr << "Error in getSelectedFile: indexOnPage=" << indexOnPage
                << " >= pageData.size=" << pageData.size() << std::endl;
    }
    return nullptr;
}
