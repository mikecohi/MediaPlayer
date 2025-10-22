#pragma once
#include "utils/NcursesUI.h"
#include <string>
#include <vector>
#include <optional> // For returning optional results

/**
 * @class PopupView
 * @brief A simple modal popup for text input or list selection.
 */
class PopupView {
public:
    PopupView(NcursesUI* ui, int parentHeight, int parentWidth);
    ~PopupView();

    /**
     * @brief Shows a popup asking for text input.
     * @param prompt The message to display (e.g., "Enter Playlist Name:").
     * @param initialValue Optional initial text in the input field.
     * @return The text entered by the user, or std::nullopt if cancelled (ESC).
     */
    std::optional<std::string> showTextInput(const std::string& prompt, const std::string& initialValue = "");

    /**
     * @brief Shows a popup with a list of options for selection.
     * @param title The title for the popup window.
     * @param options The list of strings to display.
     * @return The index of the selected option, or std::nullopt if cancelled (ESC).
     */
    std::optional<int> showListSelection(const std::string& title, const std::vector<std::string>& options);


private:
    void drawWindow(const std::string& title);
    void clearWindow();

    NcursesUI* ui;
    WINDOW* win;
    int winHeight, winWidth;
    int parentH, parentW; // Dimensions of the parent window/screen
};