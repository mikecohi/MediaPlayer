#pragma once
#include "utils/NcursesUI.h" // Cần InputEvent

/**
 * @enum ViewId
 * @brief Defines navigation targets.
 * Each view, when handling input, will return one of these IDs
 * to tell the ViewManager where to go next.
 */
enum class ViewId {
    STAY,           // Don't navigate, stay on the current view
    MAIN_MENU,
    MEDIA_LIST,
    PLAYLIST_MENU,
    DETAIL_VIEW,
    QUIT            // Signal to exit the application
};

/**
 * @class IView
 * @brief The interface (abstract base class) for all Views.
 * This allows ViewManager to manage any view polymorphically.
 */
class IView {
public:
    virtual ~IView() = default;

    /**
     * @brief Renders the view's UI to its ncurses window.
     */
    virtual void draw() = 0;

    /**
     * @brief Processes user input.
     * @param event The input event (key or mouse) from NcursesUI.
     * @return A ViewId indicating where to navigate next.
     */
    virtual ViewId handleInput(InputEvent event) = 0;
};