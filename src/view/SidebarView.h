#pragma once
#include "utils/NcursesUI.h"
#include <vector>
#include <string>

// Forward declare
enum class AppMode; // Use enum from UIManager.h if possible
enum class FocusArea;

class SidebarView {
public:
    SidebarView(NcursesUI* ui, WINDOW* win);
    void draw(bool hasFocus);
    AppMode handleInput(InputEvent event);
    AppMode handleMouse(int localY, int localX);
    bool shouldExit() const;

private: // <-- PHẦN PRIVATE
    /**
     * @brief Determines the AppMode based on the currently selected option.
     * @return The AppMode corresponding to selectedOption.
     */
    AppMode getCurrentModeSelection() const; // <-- THÊM KHAI BÁO NÀY

    NcursesUI* ui;
    WINDOW* win;
    int selectedOption;
    bool exitFlag;
    std::vector<std::string> options;
};