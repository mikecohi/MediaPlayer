#include "app/App.h"
#include "utils/NcursesUI.h"
#include "utils/FileUtils.h"
#include "view/UIManager.h"
#include "controller/AppController.h"
#include <iostream>
#include <fstream> 
#include <filesystem>

#include "model/MediaManager.h" 
#include "model/PlaylistManager.h"

namespace fs = std::filesystem;

const std::string PLAYLIST_FILENAME = "playlist/playlists.json";
const std::string MEDIA_PATH = "test_media";

App::App() {}

App::~App() {
    if (appController && appController->getPlaylistManager()) {
        fs::path rootPath = FileUtils::getProjectRootPath();
        fs::path playlistPath = rootPath / PLAYLIST_FILENAME;
        appController->getPlaylistManager()->saveToFile(playlistPath.string());
    }
    std::cout << "App: Cleanup complete." << std::endl;
    // unique_ptrs handle cleanup of uiManager, appController, ui
}

bool App::init() {
    ui = std::make_unique<NcursesUI>();
    if (!ui->initScreen()) {
        std::cerr << "Failed to initialize NcursesUI!" << std::endl;
        return false;
    }

    appController = std::make_unique<AppController>();
    if (!appController->init()) {
        std::cerr << "Failed to initialize AppController!" << std::endl;
        return false;
    }

    uiManager = std::make_unique<UIManager>(ui.get(), appController.get());
    if (!uiManager->init()) {
        std::cerr << "Failed to initialize UIManager!" << std::endl;
        return false;
    }

    fs::path rootPath = FileUtils::getProjectRootPath();
    fs::path mediaPath = rootPath / MEDIA_PATH;

    // --- 1️⃣ LOAD MEDIA TRONG THƯ MỤC CỤC BỘ ---
    std::cout << "App: Loading initial media from " << mediaPath.string() << " ..." << std::endl;
    appController->getMediaManager()->loadFromDirectory(mediaPath.string());

    // --- LOGGING ---
    if (appController && appController->getMediaManager()) {
        int count = appController->getMediaManager()->getTotalFileCount();
        std::cout << "DEBUG App::init: MediaManager reports "
                  << count << " files AFTER loading from " << mediaPath.string() << "." << std::endl;
    } else {
        std::cout << "DEBUG App::init: AppController or MediaManager is NULL!" << std::endl;
    }
    std::cout << "App: Media loading complete." << std::endl;

    // --- 2️⃣ LOAD USB MEDIA (NẾU CÓ) TRƯỚC KHI LOAD PLAYLIST ---
    std::cout << "App: Checking for USB media..." << std::endl;
    if (appController->loadUSBLibrary()) {
        int usbCount = appController->getUSBMediaManager()->getTotalFileCount();
        std::cout << "App: USB media loaded successfully. Found " << usbCount << " files." << std::endl;
    } else {
        std::cout << "App: No USB media detected or failed to load." << std::endl;
    }

    // --- 3️⃣ LOAD PLAYLIST SAU KHI CẢ HAI MEDIA MANAGER ĐÃ SẴN SÀNG ---
    fs::path playlistPath = rootPath / PLAYLIST_FILENAME;
    if (appController && appController->getPlaylistManager()) {
        std::cout << "App: Loading playlists from " << playlistPath.string() << "..." << std::endl;
        appController->getPlaylistManager()->loadFromFile(playlistPath.string());
    }

    return true;
}

void App::run() {
    if (uiManager) {
        uiManager->run();
    }
}
