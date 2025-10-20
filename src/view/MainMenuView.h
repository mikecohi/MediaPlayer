#pragma once
#include "view/IView.h"
#include "utils/NcursesUI.h"
#include <vector>
#include <string>

/**
 * @class MainMenuView
 * @brief Implements the IView interface for the main menu screen.
 */
class MainMenuView : public IView {
public:
    explicit MainMenuView(NcursesUI* ui);
    ~MainMenuView();

    void draw() override;
    ViewId handleInput(InputEvent event) override;

private:
    /**
     * @brief Helper function to print the menu options.
     */
    void printMenu();

    NcursesUI* ui; // Non-owning pointer
    WINDOW* win;   // The ncurses window for this view
    
    std::vector<std::string> options;
    int selectedOption;
};