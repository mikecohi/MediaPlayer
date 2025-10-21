#include "view/UIManager.h"
#include <iostream>
#include <cmath>
#include <tuple> // Needed for std::ignore

// Include child views and controllers
#include "controller/AppController.h"
#include "controller/MediaController.h"
#include "view/SidebarView.h"
#include "view/BottomBarView.h"
#include "view/MainFileView.h"
#include "view/MainPlaylistView.h"
#include "model/MediaFile.h"

UIManager::UIManager(NcursesUI* ui, AppController* controller)
    : ui(ui), appController(controller), isRunning(false),
      screenH(0), screenW(0), mainHeight(0), mainWidth(0),
      sidebarWin(nullptr), mainWin(nullptr), bottomWin(nullptr),
      sidebarWidth(20), bottomBarHeight(5),
      currentMode(AppMode::FILE_BROWSER),
      currentFocus(FocusArea::SIDEBAR),
      // Initialize redraw flags
      needsRedrawSidebar(true), // Redraw initially
      needsRedrawMain(true)     // Redraw initially
{
    if (appController == nullptr) {
        std::cerr << "CRITICAL: UIManager started with null AppController!" << std::endl;
    }
}

UIManager::~UIManager() {
    // ncurses windows are deleted automatically if created with newwin
    // unique_ptrs handle child views
    // Ensure cleanup of windows explicitly if needed, but not strictly required
    // if (sidebarWin) delwin(sidebarWin);
    // if (mainWin) delwin(mainWin);
    // if (bottomWin) delwin(bottomWin);
}

bool UIManager::init() {
    if (ui == nullptr) return false;
    ui->getScreenDimensions(screenH, screenW);

    mainHeight = screenH - bottomBarHeight;
    mainWidth = screenW - sidebarWidth;

    if (mainHeight < 10 || mainWidth < 20) {
        std::cerr << "Terminal is too small!" << std::endl;
        return false;
    }

    // Create ncurses windows
    sidebarWin = newwin(mainHeight, sidebarWidth, 0, 0);
    mainWin = newwin(mainHeight, mainWidth, 0, sidebarWidth);
    bottomWin = newwin(bottomBarHeight, screenW, screenH - bottomBarHeight, 0);

    if (!sidebarWin || !mainWin || !bottomWin) {
         std::cerr << "Failed to create ncurses windows!" << std::endl;
         // Clean up partially created windows if necessary
         if(sidebarWin) delwin(sidebarWin);
         if(mainWin) delwin(mainWin);
         return false;
    }

    // Initialize child views, passing necessary models/controllers
    sidebarView = std::make_unique<SidebarView>(ui, sidebarWin);
    bottomBarView = std::make_unique<BottomBarView>(
        ui,
        appController->getMediaPlayer(),
        bottomWin
    );

    // Set the initial main view (File Browser)
    switchMainView(AppMode::FILE_BROWSER);

    return true; // Initialization successful
}

void UIManager::run() {
    isRunning = true;
    // No initial drawAll() here

    while (isRunning) {
        InputEvent event = ui->getInput(); // Will timeout or return input

        // Process input if any occurred
        if (event.type != InputEvent::UNKNOWN) {
            handleInput(event); // This might set redraw flags
        }

        // --- OPTIMIZED DRAWING LOGIC ---
        // Only redraw parts that need updating
        if (needsRedrawSidebar && sidebarView) {
            sidebarView->draw(currentFocus == FocusArea::SIDEBAR);
            needsRedrawSidebar = false; // Reset flag after drawing
        } else if (sidebarView) {
             // Still need to call wnoutrefresh if not redrawing fully
             // This might not be necessary if draw() always calls it
             // wnoutrefresh(sidebarWin); // Optional: ensure it's copied to virt scr
        }


        if (needsRedrawMain && mainAreaView) {
            mainAreaView->draw(currentFocus);
            needsRedrawMain = false; // Reset flag after drawing
        } else if (mainAreaView) {
             // wnoutrefresh(mainWin); // Optional
        }

        // Bottom bar *always* redraws for time/progress updates
        if (bottomBarView) {
            bottomBarView->draw(currentFocus == FocusArea::BOTTOM_BAR);
        }

        // Update the physical screen once with all changes
        doupdate();
    }
}

// Helper to check if global coordinates are inside a window
bool is_mouse_in_window(WINDOW* win, int globalY, int globalX) {
    // ... (implementation remains the same) ...
    if (!win) return false;
    int begy = getbegy(win); int begx = getbegx(win);
    int maxy = begy + getmaxy(win); int maxx = begx + getmaxx(win);
    return (globalY >= begy && globalY < maxy && globalX >= begx && globalX < maxx);
}

WINDOW* UIManager::getWindowAt(int globalY, int globalX, int& localY, int& localX) {
    // ... (implementation remains the same) ...
    if (is_mouse_in_window(sidebarWin, globalY, globalX)) { localY = globalY - getbegy(sidebarWin); localX = globalX - getbegx(sidebarWin); return sidebarWin; }
    if (is_mouse_in_window(mainWin, globalY, globalX)) { localY = globalY - getbegy(mainWin); localX = globalX - getbegx(mainWin); return mainWin; }
    if (is_mouse_in_window(bottomWin, globalY, globalX)) { localY = globalY - getbegy(bottomWin); localX = globalX - getbegx(bottomWin); return bottomWin; }
    return nullptr;
}

void UIManager::handleInput(InputEvent event) {
     if (event.type == InputEvent::UNKNOWN) {
         return; // Timeout occurred, no user input to process
     }

     FocusArea oldFocus = currentFocus; // Store previous focus

     // --- MOUSE HANDLING ---
     if (event.type == InputEvent::MOUSE) {
        int localY, localX;
        WINDOW* clickedWin = getWindowAt(event.mouseY, event.mouseX, localY, localX);

        if (clickedWin == sidebarWin && sidebarView) {
            currentFocus = FocusArea::SIDEBAR;
            AppMode newMode = sidebarView->handleMouse(localY, localX);
            if (newMode != currentMode) {
                switchMainView(newMode); // Will set needsRedrawMain
            }
            needsRedrawSidebar = true; // Sidebar selection changed visually
        }
        else if (clickedWin == mainWin && mainAreaView) {
            // Determine sub-focus based on click position
            FocusArea oldMainFocus = currentFocus; // Store focus before handling
            if (localX < mainWidth / 2) currentFocus = FocusArea::MAIN_LIST;
            else currentFocus = FocusArea::MAIN_DETAIL;

            mainAreaView->handleMouse(localY, localX); // Let the view update its state
            needsRedrawMain = true; // Assume main view needs redraw after mouse
        }
        else if (clickedWin == bottomWin && bottomBarView) {
            currentFocus = FocusArea::BOTTOM_BAR;
            bool actionTaken = bottomBarView->handleMouse(localY, localX);
            if (actionTaken && appController && appController->getMediaController()) {
                 appController->getMediaController()->pauseOrResume();
                 // Bottom bar redraws anyway, no flag needed
            } else {
                 needsRedrawSidebar = true; // Redraw other panels if click was in bottom but no action
                 needsRedrawMain = true;
            }
        }
     }

     // --- KEYBOARD HANDLING ---
     else if (event.type == InputEvent::KEYBOARD) {
        if (event.key == 9) { // Tab key
            if (currentFocus == FocusArea::SIDEBAR) currentFocus = FocusArea::MAIN_LIST;
            else if (currentFocus == FocusArea::MAIN_LIST) currentFocus = FocusArea::MAIN_DETAIL;
            else if (currentFocus == FocusArea::MAIN_DETAIL) currentFocus = FocusArea::BOTTOM_BAR;
            else if (currentFocus == FocusArea::BOTTOM_BAR) currentFocus = FocusArea::SIDEBAR;
            // Focus change requires redrawing affected panels for highlight
            needsRedrawSidebar = true;
            needsRedrawMain = true;
            return; // Only change focus
        }

        // Delegate keyboard input based on focus
        switch (currentFocus) {
            case FocusArea::SIDEBAR:
                if (sidebarView) {
                    AppMode oldMode = currentMode;
                    AppMode newMode = sidebarView->handleInput(event);
                    if (newMode != oldMode) {
                        switchMainView(newMode); // Sets needsRedrawMain
                    }
                    if (sidebarView->shouldExit()) isRunning = false;
                    needsRedrawSidebar = true; // Always redraw sidebar after input
                }
                break;

            case FocusArea::MAIN_LIST:
                 if (mainAreaView) {
                    mainAreaView->handleInput(event, currentFocus);
                    needsRedrawMain = true; // Assume main needs redraw after key input
                 }
                 // Special handling for Enter key to play
                 if (event.key == 10) {
                     MediaFile* selectedFile = nullptr;
                     // ... (logic to get selectedFile) ...
                     if (currentMode == AppMode::FILE_BROWSER || currentMode == AppMode::USB_BROWSER) { /* getSelectedFile */ MainFileView* fv=dynamic_cast<MainFileView*>(mainAreaView.get()); if(fv) selectedFile=fv->getSelectedFile();}
                     else if (currentMode == AppMode::PLAYLISTS) { /* getSelectedTrack */ MainPlaylistView* pv=dynamic_cast<MainPlaylistView*>(mainAreaView.get()); if(pv) selectedFile=pv->getSelectedTrack();}


                     if (selectedFile && appController && appController->getMediaController()) {
                         appController->getMediaController()->playTrack(selectedFile);
                         // Bottom bar will update on next draw cycle
                     } else {
                         if (ui) flash(); // Visual cue for failed play
                     }
                 }
                 break; // Added missing break

            case FocusArea::MAIN_DETAIL:
                 if (mainAreaView) {
                    mainAreaView->handleInput(event, currentFocus);
                    needsRedrawMain = true; // Assume main needs redraw after key input
                 }
                 // TODO: Handle Enter/Click on [Edit Metadata] etc.
                break;

            case FocusArea::BOTTOM_BAR:
                if (event.key == ' ' || event.key == 'p') {
                    if (appController && appController->getMediaController()) {
                        appController->getMediaController()->pauseOrResume();
                        // Bottom bar updates on next cycle
                    }
                }
                // (TODO: Handle << >> Vol- Vol+ keys)
                // Redraw other panels in case focus highlight needs update
                needsRedrawSidebar = true;
                needsRedrawMain = true;
                break;
        }
     }

     // If focus changed due to mouse or keyboard logic (other than Tab), redraw relevant panels
     if(oldFocus != currentFocus) {
         needsRedrawSidebar = true;
         needsRedrawMain = true;
     }
}

void UIManager::switchMainView(AppMode newMode) {
    // Only switch if the mode actually changes
    if (newMode == currentMode && mainAreaView != nullptr) return;

    // Store the old focus to potentially restore later if needed
    // FocusArea oldMainFocus = (currentFocus == FocusArea::MAIN_LIST || currentFocus == FocusArea::MAIN_DETAIL) ? currentFocus : FocusArea::MAIN_LIST;

    currentMode = newMode;

    // Create the appropriate main area view
    if (newMode == AppMode::FILE_BROWSER || newMode == AppMode::USB_BROWSER) {
        mainAreaView = std::make_unique<MainFileView>(
            ui,
            mainWin,
            appController->getMediaManager()
        );
        // TODO Giai đoạn 4: If USB_BROWSER, call mediaManager->loadFromDirectory("/path/to/usb")? Or pass path?
    } else if (newMode == AppMode::PLAYLISTS) {
        mainAreaView = std::make_unique<MainPlaylistView>(
            ui,
            mainWin,
            appController->getPlaylistManager()
        );
    } else {
        mainAreaView = nullptr; // Should not happen with current modes
    }

    needsRedrawMain = true; // New main view needs to be drawn
    // Optional: Reset focus to the list part of the new main view
    // currentFocus = FocusArea::MAIN_LIST;
    // needsRedrawSidebar = true; // Redraw sidebar if focus changes
}