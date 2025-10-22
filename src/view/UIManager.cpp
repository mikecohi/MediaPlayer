#include "view/UIManager.h"
#include <iostream>
#include <cmath>
#include <tuple>
#include <vector> // Needed for popup options

// Include child views, controllers, and models
#include "controller/AppController.h"
#include "controller/MediaController.h"
#include "controller/PlaylistController.h" // Needed for playlist actions
#include "view/SidebarView.h"
#include "view/BottomBarView.h"
#include "view/MainFileView.h"
#include "view/MainPlaylistView.h"
#include "model/MediaFile.h"
#include "model/Playlist.h"          // Needed for playlist actions
#include "model/PlaylistManager.h"   // Needed for popup list
#include "view/PopupView.h"          // <-- Include PopupView

UIManager::UIManager(NcursesUI* ui, AppController* controller)
    : ui(ui), appController(controller), isRunning(false),
      screenH(0), screenW(0), mainHeight(0), mainWidth(0),
      sidebarWin(nullptr), mainWin(nullptr), bottomWin(nullptr),
      sidebarWidth(20), bottomBarHeight(5),
      currentMode(AppMode::FILE_BROWSER),
      currentFocus(FocusArea::SIDEBAR),
      // Initialize redraw flags
      needsRedrawSidebar(true),
      needsRedrawMain(true),
      popup(nullptr) // Initialize popup pointer
{
    if (appController == nullptr) {
        std::cerr << "CRITICAL: UIManager started with null AppController!" << std::endl;
    }
}

UIManager::~UIManager() {
    // Windows are managed by ncurses, unique_ptrs manage views
}

bool UIManager::init() {
    if (ui == nullptr) return false;
    ui->getScreenDimensions(screenH, screenW);

    mainHeight = screenH - bottomBarHeight;
    mainWidth = screenW - sidebarWidth;

    if (mainHeight < 10 || mainWidth < 20) { /* Terminal too small */ return false; }

    sidebarWin = newwin(mainHeight, sidebarWidth, 0, 0);
    mainWin = newwin(mainHeight, mainWidth, 0, sidebarWidth);
    bottomWin = newwin(bottomBarHeight, screenW, screenH - bottomBarHeight, 0);

    if (!sidebarWin || !mainWin || !bottomWin) { /* Failed win create */ return false; }

    // Initialize child views
    sidebarView = std::make_unique<SidebarView>(ui, sidebarWin);
    bottomBarView = std::make_unique<BottomBarView>(ui, appController->getMediaPlayer(), bottomWin);

    switchMainView(AppMode::FILE_BROWSER);

    // Initialize PopupView (lazy init is also fine)
    popup = std::make_unique<PopupView>(ui, screenH, screenW);


    return true;
}

void UIManager::run() {
    isRunning = true;
    while (isRunning) {
        InputEvent event = ui->getInput();

        if (event.type != InputEvent::UNKNOWN) {
            handleInput(event);
        }

        // --- OPTIMIZED DRAWING ---
        if (needsRedrawSidebar && sidebarView) {
            sidebarView->draw(currentFocus == FocusArea::SIDEBAR);
            needsRedrawSidebar = false;
        }
        if (needsRedrawMain && mainAreaView) {
            mainAreaView->draw(currentFocus);
            needsRedrawMain = false;
        }
        if (bottomBarView) {
            bottomBarView->draw(currentFocus == FocusArea::BOTTOM_BAR);
        }
        doupdate(); // Update physical screen
    }
}

// Helper: Check if mouse is in window
bool is_mouse_in_window(WINDOW* win, int globalY, int globalX) {
    if (!win) return false; int begy=getbegy(win), begx=getbegx(win), maxy=begy+getmaxy(win), maxx=begx+getmaxx(win);
    return (globalY >= begy && globalY < maxy && globalX >= begx && globalX < maxx);
}
// Helper: Get window at coordinates
WINDOW* UIManager::getWindowAt(int gY, int gX, int& lY, int& lX) {
    if (is_mouse_in_window(sidebarWin, gY, gX)) { lY=gY-getbegy(sidebarWin); lX=gX-getbegx(sidebarWin); return sidebarWin; }
    if (is_mouse_in_window(mainWin, gY, gX)) { lY=gY-getbegy(mainWin); lX=gX-getbegx(mainWin); return mainWin; }
    if (is_mouse_in_window(bottomWin, gY, gX)) { lY=gY-getbegy(bottomWin); lX=gX-getbegx(bottomWin); return bottomWin; }
    return nullptr;
}


void UIManager::handleInput(InputEvent event) {
     if (event.type == InputEvent::UNKNOWN) return;

     FocusArea oldFocus = currentFocus;
     MainAreaAction action = MainAreaAction::NONE; // Action returned by child view

     // --- MOUSE HANDLING ---
     if (event.type == InputEvent::MOUSE) {
        int localY, localX;
        WINDOW* clickedWin = getWindowAt(event.mouseY, event.mouseX, localY, localX);

        if (clickedWin == sidebarWin && sidebarView) {
            currentFocus = FocusArea::SIDEBAR;
            AppMode newMode = sidebarView->handleMouse(localY, localX);
            if (newMode != currentMode) switchMainView(newMode);
            if (sidebarView->shouldExit()) isRunning = false;
            needsRedrawSidebar = true;
        }
        else if (clickedWin == mainWin && mainAreaView) {
            if (localX < mainWidth / 2) currentFocus = FocusArea::MAIN_LIST;
            else currentFocus = FocusArea::MAIN_DETAIL;
            // Get action from mouse handler
            action = mainAreaView->handleMouse(localY, localX);
            needsRedrawMain = true;
        }
        else if (clickedWin == bottomWin && bottomBarView) {
            currentFocus = FocusArea::BOTTOM_BAR;
            bool pauseAction = bottomBarView->handleMouse(localY, localX);
            if (pauseAction && appController && appController->getMediaController()) {
                 appController->getMediaController()->pauseOrResume();
            } else { // Click elsewhere in bottom bar, ensure other panels redraw focus
                 needsRedrawSidebar = true;
                 needsRedrawMain = true;
            }
        }
     }

     // --- KEYBOARD HANDLING ---
     else if (event.type == InputEvent::KEYBOARD) {
        if (event.key == 9) { // Tab key
            // ... (Tab focus logic) ...
            if (currentFocus == FocusArea::SIDEBAR) currentFocus = FocusArea::MAIN_LIST; else if (currentFocus == FocusArea::MAIN_LIST) currentFocus = FocusArea::MAIN_DETAIL; else if (currentFocus == FocusArea::MAIN_DETAIL) currentFocus = FocusArea::BOTTOM_BAR; else if (currentFocus == FocusArea::BOTTOM_BAR) currentFocus = FocusArea::SIDEBAR;
            needsRedrawSidebar = true; needsRedrawMain = true;
            return;
        }

        switch (currentFocus) {
            case FocusArea::SIDEBAR:
                if (sidebarView) { /* ... Sidebar key handling ... */ AppMode oldM=currentMode; AppMode newM=sidebarView->handleInput(event); if(newM != oldM) switchMainView(newM); if(sidebarView->shouldExit()) isRunning=false; needsRedrawSidebar=true; }
                break;

            case FocusArea::MAIN_LIST:
                 if (mainAreaView) {
                     action = mainAreaView->handleInput(event, currentFocus); // Handle Up/Down etc.
                     needsRedrawMain = true;
                 }
                 // Special handling for Enter key to play
                 if (event.key == 10) { /* ... Play track logic ... */ MediaFile* sf=nullptr; if(currentMode==AppMode::FILE_BROWSER||currentMode==AppMode::USB_BROWSER){MainFileView* fv=dynamic_cast<MainFileView*>(mainAreaView.get()); if(fv) sf=fv->getSelectedFile();} else if(currentMode==AppMode::PLAYLISTS){MainPlaylistView* pv=dynamic_cast<MainPlaylistView*>(mainAreaView.get()); if(pv) sf=pv->getSelectedTrack();} if(sf&&appController&&appController->getMediaController()){appController->getMediaController()->playTrack(sf);} else{if(ui)flash();}}
                 break;

            case FocusArea::MAIN_DETAIL:
                 if (mainAreaView) {
                     action = mainAreaView->handleInput(event, currentFocus); // Handle button activation?
                     needsRedrawMain = true;
                 }
                break;

            case FocusArea::BOTTOM_BAR:
                 /* ... Handle Space/P for pause/resume ... */ if(event.key==' '||event.key=='p'){if(appController&&appController->getMediaController()){appController->getMediaController()->pauseOrResume();}}
                needsRedrawSidebar = true; needsRedrawMain = true; // Redraw focus highlights
                break;
        }
     } // End Keyboard Handling


     // --- HANDLE ACTION RETURNED BY MAIN VIEW (From Mouse or Keyboard) ---
     switch (action) {
         case MainAreaAction::CREATE_PLAYLIST:
             showCreatePlaylistPopup();
             break;
         case MainAreaAction::DELETE_PLAYLIST:
             { // Scope for variables
                 Playlist* selectedPlaylist = nullptr;
                 if (currentMode == AppMode::PLAYLISTS) {
                     MainPlaylistView* plView = dynamic_cast<MainPlaylistView*>(mainAreaView.get());
                     // Need getSelectedPlaylist() function in MainPlaylistView
                     // Example placeholder: Assume selection index is correct
                     if (plView && appController && appController->getPlaylistManager()) {
                          auto playlists = appController->getPlaylistManager()->getAllPlaylists();
                          int index = plView->getSelectedPlaylistIndex(); // Need this func
                          if (index >= 0 && static_cast<size_t>(index) < playlists.size()) {
                               selectedPlaylist = playlists[index];
                          }
                     }
                 }
                 if (selectedPlaylist && appController && appController->getPlaylistController()) {
                     bool deleted = appController->getPlaylistController()->deletePlaylist(selectedPlaylist->getName());
                     if (deleted) needsRedrawMain = true; // Update list
                     else flash(); // Indicate error
                 } else { flash();}
             }
             break;
         case MainAreaAction::REMOVE_TRACK_FROM_PLAYLIST:
             { // Scope for variables
                 MediaFile* selectedTrack = nullptr;
                 Playlist* currentPlaylist = nullptr;
                 if (currentMode == AppMode::PLAYLISTS) {
                    MainPlaylistView* plView = dynamic_cast<MainPlaylistView*>(mainAreaView.get());
                    if(plView && appController && appController->getPlaylistManager()) {
                       selectedTrack = plView->getSelectedTrack();
                       // Need getSelectedPlaylist() function in MainPlaylistView
                       int index = plView->getSelectedPlaylistIndex(); // Need this func
                       auto playlists = appController->getPlaylistManager()->getAllPlaylists();
                       if (index >= 0 && static_cast<size_t>(index) < playlists.size()) {
                            currentPlaylist = playlists[index];
                       }
                    }
                 }
                 if (selectedTrack && currentPlaylist && appController && appController->getPlaylistController()) {
                      bool removed = appController->getPlaylistController()->removeTrackFromPlaylist(selectedTrack, currentPlaylist);
                      if(removed) needsRedrawMain = true; // Update track list
                      else flash();
                 } else { flash();}
             }
            break;
         case MainAreaAction::ADD_TRACK_TO_PLAYLIST:
             { // Scope for variable
                 MediaFile* selectedFile = nullptr;
                 if (currentMode == AppMode::FILE_BROWSER || currentMode == AppMode::USB_BROWSER) {
                     MainFileView* fileView = dynamic_cast<MainFileView*>(mainAreaView.get());
                     if(fileView) selectedFile = fileView->getSelectedFile();
                 }
                 if (selectedFile) {
                     showAddToPlaylistPopup(selectedFile);
                 } else { flash();}
             }
             break;
          case MainAreaAction::EDIT_METADATA:
              // TODO: Implement Edit Metadata Popup
              flash(); // Placeholder feedback
              break;
         case MainAreaAction::NONE:
         default:
             // No action needed from UIManager
             break;
     }

     // Redraw if focus changed
     if(oldFocus != currentFocus) {
         needsRedrawSidebar = true;
         needsRedrawMain = true;
     }
}


// --- NEW POPUP HELPER METHODS ---

void UIManager::showCreatePlaylistPopup() {
    if (!popup) { // Should be initialized in init() now
         std::cerr << "Error: PopupView not initialized!" << std::endl;
         return;
    }
    std::optional<std::string> name = popup->showTextInput("Enter New Playlist Name:");

    // Force redraw of underlying UI after popup closes
    needsRedrawSidebar = true;
    needsRedrawMain = true;
    // Bottom bar redraws anyway

    if (name.has_value() && !name.value().empty() && appController && appController->getPlaylistController()) {
        bool success = appController->getPlaylistController()->createPlaylist(name.value());
        if (!success) {
            flash(); // Simple visual feedback for failure (e.g., duplicate name)
        }
        // Always redraw main to show the updated playlist list (even on failure)
        needsRedrawMain = true;
    }
}

void UIManager::showAddToPlaylistPopup(MediaFile* fileToAdd) {
     if (!fileToAdd || !appController || !appController->getPlaylistManager()) return;

     // Get playlist names for options
     std::vector<Playlist*> playlists = appController->getPlaylistManager()->getAllPlaylists();
     std::vector<std::string> options;
     options.reserve(playlists.size());
     for(const auto* pl : playlists) {
         if(pl) options.push_back(pl->getName());
     }
     // TODO: Optional: Add "[Create New Playlist]" option here

     if (options.empty()) {
          // TODO: Maybe show a different popup saying "No playlists exist..."
          flash(); // Simple feedback
          return;
     }

     if (!popup) { // Should be initialized
          std::cerr << "Error: PopupView not initialized!" << std::endl;
          return;
     }
     std::optional<int> selectedIndex = popup->showListSelection("Add to Playlist:", options);

     // Force redraw after popup
     needsRedrawSidebar = true;
     needsRedrawMain = true;

     if (selectedIndex.has_value()) {
         int index = selectedIndex.value();
         // TODO: Handle "[Create New Playlist]" selection if added

         // Ensure index is valid for the playlists vector
         if (index >= 0 && static_cast<size_t>(index) < playlists.size()) {
             Playlist* selectedPlaylist = playlists[index];
             if (selectedPlaylist && appController->getPlaylistController()) {
                  bool added = appController->getPlaylistController()->addTrackToPlaylist(fileToAdd, selectedPlaylist);
                  if (!added) flash(); // Feedback if adding failed (e.g., duplicate)
                  // No need to set redraw flag here, adding track doesn't change visible list immediately
             }
         }
     }
}

// --- switchMainView remains the same ---
void UIManager::switchMainView(AppMode newMode) {
    if (newMode == currentMode && mainAreaView != nullptr) return;
    currentMode = newMode;
    if (newMode == AppMode::FILE_BROWSER || newMode == AppMode::USB_BROWSER) {
        mainAreaView = std::make_unique<MainFileView>(ui, mainWin, appController->getMediaManager());
    } else if (newMode == AppMode::PLAYLISTS) {
        mainAreaView = std::make_unique<MainPlaylistView>(ui, mainWin, appController->getPlaylistManager());
    } else { mainAreaView = nullptr; }
    needsRedrawMain = true;
}