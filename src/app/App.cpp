#include "app/App.h"
#include "utils/NcursesUI.h"
#include "view/UIManager.h" 
#include "controller/AppController.h" 
#include <iostream>

// ===================================
// SỬA LỖI 2: Bổ sung các include đầy đủ
// ===================================
#include "model/MediaManager.h"
#include "model/PlaylistManager.h"
// (Chúng ta không cần MediaPlayer.h ở đây)


App::App() {
    // Constructor
}

App::~App() {
    // Destructor
}

bool App::init() {
    // 1. Initialize Ncurses
    ui = std::make_unique<NcursesUI>();
    if (!ui->initScreen()) {
        std::cerr << "Failed to initialize NcursesUI!" << std::endl;
        return false;
    }

    // 2. Initialize "bộ não" (AppController)
    appController = std::make_unique<AppController>();
    if (!appController->init()) {
        std::cerr << "Failed to initialize AppController!" << std::endl;
        return false;
    }

    // 3. Initialize "bộ mặt" (UIManager)
    uiManager = std::make_unique<UIManager>(ui.get(), appController.get()); 
    if (!uiManager->init()) {
        std::cerr << "Failed to initialize UIManager!" << std::endl;
        return false;
    }
    
    // --- GIAI ĐOẠN 3: NẠP DỮ LIỆU THẬT KHI KHỞI ĐỘNG ---
    std::cout << "App: Loading initial media..." << std::endl;
    
    // Dòng này giờ đã hợp lệ vì App.cpp đã include MediaManager.h
    appController->getMediaManager()->loadFromDirectory("/home/quynhmai/mock/MediaPlayer/test_media");
    
    std::cout << "App: Media loading complete." << std::endl;

    return true;
}

void App::run() {
    if (uiManager) {
        uiManager->run();
    }
}