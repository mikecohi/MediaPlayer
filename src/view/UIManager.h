#pragma once
#include "utils/NcursesUI.h" // Needs InputEvent
#include <string>
#include <vector>
#include <memory>

// Forward declarations
class SidebarView;
class BottomBarView;
class IMainAreaView;
class AppController;
class MediaFile; 
class PopupView; 

// Enums
enum class AppMode {
    FILE_BROWSER,
    PLAYLISTS,
    USB_BROWSER
};

enum class FocusArea {
    SIDEBAR,
    MAIN_LIST,
    MAIN_DETAIL,
    BOTTOM_BAR
};

class UIManager {
public:
    explicit UIManager(NcursesUI* ui, AppController* controller);
    ~UIManager();
    bool init();
    void run();

private:
    void handleInput(InputEvent event);
    //void drawAll();
    void switchMainView(AppMode newMode);
    WINDOW* getWindowAt(int globalY, int globalX, int& localY, int& localX);

    // Order matches the constructor initializer list for -Wreorder
    NcursesUI* ui;
    AppController* appController;
    bool isRunning;

    int screenH, screenW;
    int mainHeight, mainWidth;

    WINDOW *sidebarWin;
    WINDOW *mainWin;
    WINDOW *bottomWin; 
    
    int sidebarWidth;
    int bottomBarHeight;

    AppMode currentMode;
    FocusArea currentFocus;

    std::unique_ptr<SidebarView> sidebarView;
    std::unique_ptr<BottomBarView> bottomBarView;
    std::unique_ptr<IMainAreaView> mainAreaView;

    bool needsRedrawSidebar;
    bool needsRedrawMain;

    // --- ADD HELPER METHODS FOR POPUPS ---
    void showCreatePlaylistPopup();
    void showAddToPlaylistPopup(MediaFile* fileToAdd);
    
    // Add PopupView member
    std::unique_ptr<PopupView> popup; 

};