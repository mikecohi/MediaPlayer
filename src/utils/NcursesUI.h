#pragma once
#include <ncurses.h>
#include <string>

/**
 * @struct InputEvent
 * @brief A simple structure to hold input data (keyboard or mouse).
 */
struct InputEvent {
    enum EventType { KEYBOARD, MOUSE, UNKNOWN };
    
    EventType type = UNKNOWN;
    int key = 0;       // Holds the character or KEY_ value (e.g., KEY_UP)
    int mouseX = 0;    // Mouse X coordinate
    int mouseY = 0;    // Mouse Y coordinate
};

/**
 * @class NcursesUI
 * @brief A C++ wrapper for the Ncurses library.
 * This class manages the screen initialization, window creation,
 * and input handling in a simple, object-oriented way.
 */
class NcursesUI {
public:
    /**
     * @brief Constructor.
     */
    NcursesUI();

    /**
     * @brief Destructor. Automatically calls closeScreen().
     */
    ~NcursesUI();

    /**
     * @brief Initializes the ncurses screen.
     * Sets up colors, keyboard modes (cbreak, noecho), and mouse support.
     * @return true on success, false on failure.
     */
    bool initScreen();

    /**
     * @brief Closes the ncurses screen and restores the terminal.
     */
    void closeScreen();

    /**
     * @brief Creates a new, bordered window.
     * @param y The top-left Y coordinate (row).
     * @param x The top-left X coordinate (column).
     * @param height The height of the window.
     * @param width The width of the window.
     * @param title An optional title to display on the top border.
     * @return A pointer to the newly created WINDOW.
     */
    WINDOW* drawWindow(int y, int x, int height, int width, const std::string& title = "");

    /**
     * @brief Prints text at a specific position inside a window.
     * @param win The window to print in.
     * @param y The local Y coordinate (row) inside the window.
     * @param x The local X coordinate (column) inside the window.
     * @param text The string to print.
     */
    void printText(WINDOW* win, int y, int x, const std::string& text);

    /**
     * @brief Waits for and retrieves a single input event (key or mouse).
     * This is a blocking call.
     * @return An InputEvent structure.
     */
    InputEvent getInput();

    /**
     * @brief Refreshes the main screen to show changes.
     */
    void refreshScreen();

    /**
     * @brief Clears the main screen.
     */
    void clearScreen();

    /**
     * @brief Gets the screen dimensions.
     * @param outHeight Reference to store the height.
     * @param outWidth Reference to store the width.
     */
    void getScreenDimensions(int& outHeight, int& outWidth);

private:
    bool isInitialized;
    int screenHeight;
    int screenWidth;
};