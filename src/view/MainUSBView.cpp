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
}

// --- Kiểm tra trạng thái USB ---
void MainUSBView::updateUSBStatus() {
    if (!appController) return;
    mediaManager = appController->getMediaManager();
    usbConnected = appController->loadUSBLibrary(); // Thử load, nếu thành công thì có USB
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
    int availableLines = height - 4 - 3; // chừa 3 dòng cuối cho nút
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

    // --- File list ---
    auto filesOnPage = mediaManager->getPage(filePage, itemsPerPage);
    for (size_t i = 0; i < filesOnPage.size(); ++i) {
        int y = 4 + i;
        if (focus == FocusArea::MAIN_LIST && (int)i == fileSelected)
            wattron(win, A_REVERSE | A_BOLD);
        mvwprintw(win, y, 3, "%.*s", listWidth - 5, filesOnPage[i]->getFileName().c_str());
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

    if (focus == FocusArea::MAIN_DETAIL && event.key == 10) {
        if (fileExplicitlySelected && getSelectedFile() != nullptr)
            return MainAreaAction::ADD_TRACK_TO_PLAYLIST;
    }

    if (focus == FocusArea::MAIN_LIST) {
        int totalFiles = mediaManager->getTotalFileCount();
        if (event.key == KEY_DOWN)
            fileSelected = std::min(fileSelected + 1, totalFiles - 1);
        else if (event.key == KEY_UP)
            fileSelected = std::max(fileSelected - 1, 0);
        else if (event.key == KEY_NPAGE && filePage < totalPages)
            filePage++;
        else if (event.key == KEY_PPAGE && filePage > 1)
            filePage--;
        fileExplicitlySelected = true;
    }
    return MainAreaAction::NONE;
}

// --- MOUSE HANDLER ---
MainAreaAction MainUSBView::handleMouse(int y, int x) {
    if (y == reloadBtnY && x >= reloadBtnX && x < reloadBtnX + reloadBtnW) {
        appController->reloadUSBLibrary();
        updateUSBStatus();
        return MainAreaAction::NONE;
    }
    if (y == ejectBtnY && x >= ejectBtnX && x < ejectBtnX + ejectBtnW) {
        appController->ejectUSB();
        updateUSBStatus();
        return MainAreaAction::NONE;
    }
    return MainAreaAction::NONE;
}

MediaFile* MainUSBView::getSelectedFile() const {
    if (!mediaManager) return nullptr;
    auto page = mediaManager->getPage(filePage, itemsPerPage);
    if (fileSelected < 0 || fileSelected >= (int)page.size()) return nullptr;
    return page[fileSelected];
}
