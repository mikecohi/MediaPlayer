#include "view/UIManager.h"
#include <iostream>
#include <cmath>
#include <tuple> // Cần cho std::ignore

// Include các View con VÀ AppController
#include "controller/AppController.h"
#include "view/SidebarView.h"
#include "view/BottomBarView.h"
#include "view/MainFileView.h"
#include "view/MainPlaylistView.h"

// Sửa Constructor
UIManager::UIManager(NcursesUI* ui, AppController* controller)
    : ui(ui), appController(controller), isRunning(false), 
      sidebarWin(nullptr), mainWin(nullptr), bottomWin(nullptr),
      sidebarWidth(20), bottomBarHeight(5),
      currentMode(AppMode::FILE_BROWSER),
      currentFocus(FocusArea::SIDEBAR)
{
    if (appController == nullptr) {
        std::cerr << "CRITICAL: UIManager started with null AppController!" << std::endl;
    }
}

UIManager::~UIManager() {
    // ... (rỗng) ...
}

bool UIManager::init() {
    // ... (logic tính toán kích thước, newwin giữ nguyên) ...
    int screenH, screenW;
    ui->getScreenDimensions(screenH, screenW);
    int mainHeight = screenH - bottomBarHeight;
    int mainWidth = screenW - sidebarWidth;
    if (mainHeight < 10 || mainWidth < 20) return false;
    
    sidebarWin = newwin(mainHeight, sidebarWidth, 0, 0);
    mainWin = newwin(mainHeight, mainWidth, 0, sidebarWidth);
    bottomWin = newwin(bottomBarHeight, screenW, screenH - bottomBarHeight, 0); 
    
    // --- GIAI ĐOẠN 3: TIÊM DỮ LIỆU THẬT ---
    sidebarView = std::make_unique<SidebarView>(ui, sidebarWin);
    
    // Tiêm MediaPlayer THẬT vào BottomBarView
    bottomBarView = std::make_unique<BottomBarView>(
        ui, 
        appController->getMediaPlayer(), // <-- DỮ LIỆU THẬT
        bottomWin
    );
    
    // Bắt đầu ở chế độ File
    switchMainView(AppMode::FILE_BROWSER);
    
    return true;
}

// ... (hàm run() và handleInput() giữ nguyên) ...
void UIManager::run() {
    isRunning = true;
    while (isRunning) {
        drawAll();
        InputEvent event = ui->getInput();
        handleInput(event);
    }
}

void UIManager::drawAll() {
    // ... (giữ nguyên) ...
    sidebarView->draw(currentFocus == FocusArea::SIDEBAR);
    bottomBarView->draw(currentFocus == FocusArea::BOTTOM_BAR);
    mainAreaView->draw(currentFocus);
}

void UIManager::handleInput(InputEvent event) {
    // ... (logic phím Tab giữ nguyên) ...
    if (event.key == 9) { 
        if (currentFocus == FocusArea::SIDEBAR) currentFocus = FocusArea::MAIN_LIST;
        else if (currentFocus == FocusArea::MAIN_LIST) currentFocus = FocusArea::MAIN_DETAIL;
        else if (currentFocus == FocusArea::MAIN_DETAIL) currentFocus = FocusArea::BOTTOM_BAR;
        else if (currentFocus == FocusArea::BOTTOM_BAR) currentFocus = FocusArea::SIDEBAR;
        return;
    }
    
    // ... (logic switch(currentFocus) giữ nguyên) ...
    switch (currentFocus) {
        case FocusArea::SIDEBAR:
            {
                AppMode newMode = sidebarView->handleInput(event);
                if (newMode != currentMode) {
                    switchMainView(newMode);
                }
                if (sidebarView->shouldExit()) isRunning = false;
            }
            break;
        case FocusArea::MAIN_LIST:
        case FocusArea::MAIN_DETAIL:
            mainAreaView->handleInput(event, currentFocus);
            break;
        case FocusArea::BOTTOM_BAR:
            // (TODO: Gửi input đến MediaController)
            break;
    }
}


void UIManager::switchMainView(AppMode newMode) {
    if (newMode == currentMode && mainAreaView != nullptr) {
        return;
    }
    currentMode = newMode;
    
    // --- GIAI ĐOẠN 3: TIÊM DỮ LIỆU THẬT ---
    if (newMode == AppMode::FILE_BROWSER) {
        mainAreaView = std::make_unique<MainFileView>(
            ui, 
            mainWin, 
            appController->getMediaManager() // <-- DỮ LIỆU THẬT
        );
    } else if (newMode == AppMode::PLAYLISTS) {
        mainAreaView = std::make_unique<MainPlaylistView>(
            ui, 
            mainWin,
            appController->getPlaylistManager() // <-- DỮ LIỆU THẬT
        );
    }
}