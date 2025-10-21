#pragma once
#include "utils/NcursesUI.h"
#include <vector>
#include <string>

// Forward declare (tránh include vòng)
enum class AppMode;
enum class FocusArea;

class SidebarView {
public:
    SidebarView(NcursesUI* ui, WINDOW* win);
    void draw(bool hasFocus);
    AppMode handleInput(InputEvent event);
    bool shouldExit() const;

private:
    NcursesUI* ui;
    WINDOW* win;
    int selectedOption;
    bool exitFlag;
    std::vector<std::string> options;
};