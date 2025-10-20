#pragma once
#include "utils/NcursesUI.h"
#include "view/ViewManager.h"
#include <memory>

/**
 * @class App
 * @brief The main application class that owns and runs the UI loop.
 * (This is a simplified version for Giai đoạn 2 testing).
 */
class App {
public:
    App();
    ~App();

    /**
     * @brief Initializes all subsystems (Ncurses).
     * @return true on success, false on failure.
     */
    bool init();

    /**
     * @brief Runs the main application loop.
     */
    void run();

private:
    std::unique_ptr<NcursesUI> ui;
    std::unique_ptr<ViewManager> viewManager;
    bool isRunning;
};