#include "view/ViewManager.h"
#include "view/MainMenuView.h"
#include "view/MediaListView.h"
#include <iostream>

ViewManager::ViewManager() : ui(nullptr), currentView(nullptr) {
    // Constructor
}

ViewManager::~ViewManager() {
    // currentView is a unique_ptr, it will be automatically destroyed
}

void ViewManager::init(NcursesUI* ui) {
    this->ui = ui;
    if (this->ui == nullptr) {
        std::cerr << "CRITICAL: ViewManager initialized with null NcursesUI!" << std::endl;
        // In a real app, you might throw an exception
    }
}

void ViewManager::navigateTo(ViewId viewId) {
    // This switch statement is the core navigation logic
    switch (viewId) {
        case ViewId::MAIN_MENU:
            currentView = std::make_unique<MainMenuView>(ui);
            break;

        case ViewId::MEDIA_LIST:
            currentView = std::make_unique<MediaListView>(ui);
            break;
            
        case ViewId::PLAYLIST_MENU:
            // TODO: Create PlaylistMenuView in a future task
            // currentView = std::make_unique<PlaylistMenuView>(ui);
            std::cout << "PlaylistMenuView not yet implemented." << std::endl;
            // For now, just stay on the main menu
            currentView = std::make_unique<MainMenuView>(ui); 
            break;

        case ViewId::QUIT:
            currentView = nullptr; // Clear the view
            break;

        default:
            // Do nothing if ViewId::STAY or unknown
            return;
    }

    // Draw the new view immediately after navigating
    if (currentView) {
        currentView->draw();
    }
}

void ViewManager::drawCurrentView() {
    if (currentView) {
        currentView->draw();
    }
}

ViewId ViewManager::handleInput(InputEvent event) {
    if (currentView == nullptr) {
        return ViewId::QUIT; // No view, nowhere to go
    }

    // Delegate input handling to the active view
    ViewId nextViewId = currentView->handleInput(event);

    // If the view requested a change...
    if (nextViewId != ViewId::STAY) {
        navigateTo(nextViewId); // ...navigate to the new view
    }
    
    return nextViewId;
}