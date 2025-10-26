#include "app/App.h"
#include "utils/NcursesUI.h"
#include "view/UIManager.h"
#include "controller/AppController.h"
#include <iostream>
#include <fstream> 

#include "model/MediaManager.h" 
#include "model/PlaylistManager.h"

const std::string PLAYLIST_FILENAME = "//home/dung20210222/Documents/F/MediaPlayer/playlist/playlists.json";

App::App() {}
App::~App() {
    if (appController && appController->getPlaylistManager()) {
        std::cout << "App: Saving playlists to " << PLAYLIST_FILENAME << "..." << std::endl;
        // Use the constant filename
        appController->getPlaylistManager()->saveToFile(PLAYLIST_FILENAME);
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
    
    // --- LOAD MEDIA ---
    std::cout << "App: Loading initial media from ./test_media ..." << std::endl;
    std::string mediaPath = "/home/dung20210222/Documents/F/MediaPlayer/test_media";
    appController->getMediaManager()->loadFromDirectory(mediaPath);

    // --- LOGGING ---
    if (appController && appController->getMediaManager()) {
        int count = appController->getMediaManager()->getTotalFileCount();
        std::cout << "DEBUG App::init: MediaManager reports "
                  << count << " files AFTER loading from " << mediaPath << "." << std::endl;
        // Optional: Log to file
        // std::ofstream logfile("app_init.log", std::ios::app);
        // logfile << "DEBUG App::init: MediaManager reports " << count << " files AFTER loading." << std::endl;
    } else {
         std::cout << "DEBUG App::init: AppController or MediaManager is NULL after loading!" << std::endl;
    }
    // --- END LOGGING ---

    std::cout << "App: Media loading complete." << std::endl;

    // --- LOAD PLAYLISTS AFTER MEDIA ---
    if (appController && appController->getPlaylistManager()) {
        std::cout << "App: Loading playlists from " << PLAYLIST_FILENAME << "..." << std::endl;
        appController->getPlaylistManager()->loadFromFile(PLAYLIST_FILENAME);
    }

    return true;
}

void App::run() {
    if (uiManager) {
        uiManager->run();
    }
}