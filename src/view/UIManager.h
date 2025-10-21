#pragma once
#include "utils/NcursesUI.h"
#include <string>
#include <vector>
#include <memory>

// Forward declarations
class SidebarView;
class BottomBarView;
class IMainAreaView;
class AppController; 

// ===================================
// SỬA LỖI 1: Bổ sung các định nghĩa ENUM
// ===================================

// Định nghĩa các chế độ (state) của ứng dụng
enum class AppMode {
    FILE_BROWSER,
    PLAYLISTS,
    USB_BROWSER
};

// Định nghĩa khu vực đang được "focus" (để điều hướng)
enum class FocusArea {
    SIDEBAR,
    MAIN_LIST,
    MAIN_DETAIL,
    BOTTOM_BAR
};

/**
 * @class UIManager
 * @brief Manages the entire 3-pane persistent UI layout.
 */
class UIManager {
public:
    // Sửa Constructor: Nhận AppController
    explicit UIManager(NcursesUI* ui, AppController* controller); 
    ~UIManager();
    bool init(); 
    void run();

private:
    void handleInput(InputEvent event);
    void drawAll();
    void switchMainView(AppMode newMode); // <-- Giờ đã hợp lệ

    NcursesUI* ui;
    AppController* appController; 
    bool isRunning;
    
    int sidebarWidth;
    int bottomBarHeight;

    WINDOW *sidebarWin, *mainWin, *bottomWin;

    // Các dòng này giờ đã hợp lệ
    AppMode currentMode;
    FocusArea currentFocus;

    std::unique_ptr<SidebarView> sidebarView;
    std::unique_ptr<BottomBarView> bottomBarView;
    std::unique_ptr<IMainAreaView> mainAreaView;
};