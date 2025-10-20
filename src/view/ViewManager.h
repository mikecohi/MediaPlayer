#pragma once
#include <memory>
#include "utils/NcursesUI.h"
#include "view/IView.h" // Include the new interface

// Forward-declare all concrete views
class MainMenuView;
class MediaListView;

/**
 * @class ViewManager
 * @brief Manages the active view and handles view navigation.
 */
class ViewManager {
public:
    ViewManager();
    ~ViewManager();

    /**
     * @brief Initializes the ViewManager with the Ncurses wrapper.
     * @param ui A non-owning pointer to the NcursesUI instance.
     */
    void init(NcursesUI* ui);

    /**
     * @brief Navigates to a new view.
     * This will destroy the old view and create a new one.
     * @param viewId The ID of the view to navigate to.
     */
    void navigateTo(ViewId viewId);

    /**
     * @brief Calls the draw() method of the current active view.
     */
    void drawCurrentView();

    /**
     * @brief Passes user input to the current active view for handling.
     * If the view requests navigation, this handles the transition.
     * @param event The input event.
     * @return The ViewId of the *next* view, or ViewId::STAY.
     */
    ViewId handleInput(InputEvent event);

private:
    NcursesUI* ui; // Non-owning pointer
    std::unique_ptr<IView> currentView; // Smart pointer to the active view
};