#include "app/App.h"
#include "utils/NcursesUI.h"
#include "view/UIManager.h"
#include "controller/AppController.h"
#include <iostream>
#include <fstream> // For file logging if needed

#include "model/MediaManager.h" // Needed for the log below
#include "model/PlaylistManager.h"

App::App() {}
App::~App() {}

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

    std::cout << "App: Loading initial media from ./test_media ..." << std::endl;
    std::string mediaPath = "/home/quynhmai/mock/MediaPlayer/test_media";
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

    return true;
}

void App::run() {
    if (uiManager) {
        uiManager->run();
    }
}