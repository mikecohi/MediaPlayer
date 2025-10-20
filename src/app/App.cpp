#include "app/App.h"
#include <iostream>

App::App() : isRunning(false) {
    // Constructor
}

App::~App() {
    // Clean up
    if (ui) {
        ui->closeScreen();
    }
}

bool App::init() {
    // 1. Initialize the UI Engine
    ui = std::make_unique<NcursesUI>();
    if (!ui->initScreen()) {
        std::cerr << "Failed to initialize NcursesUI!" << std::endl;
        return false;
    }

    // 2. Initialize the View Manager
    viewManager = std::make_unique<ViewManager>();
    viewManager->init(ui.get()); // Pass the non-owning pointer

    isRunning = true;
    return true;
}

void App::run() {
    if (!isRunning) {
        return;
    }

    // Start at the Main Menu
    viewManager->navigateTo(ViewId::MAIN_MENU);

    // --- Main Application Loop ---
    while (isRunning) {
        // 1. Get input
        InputEvent event = ui->getInput();

        // 2. Process input
        ViewId nextView = viewManager->handleInput(event);

        // 3. Check for quit signal
        if (nextView == ViewId::QUIT) {
            isRunning = false;
        }
        
        // 4. (Views draw themselves when input is handled)
    }
}