#pragma once
#include "view/IMainAreaView.h"
#include <vector>
#include <string>

// Forward-declare (khai báo trước) Model
#include "model/PlaylistManager.h"

/**
 * @class MainPlaylistView
 * @brief Implements IMainAreaView for the "Playlist" mode.
 * This version uses REAL data from the PlaylistManager.
 */
class MainPlaylistView : public IMainAreaView {
public:
    /**
     * @brief Constructor
     * @param ui Non-owning pointer to NcursesUI.
     * @param win The ncurses window this view will draw into.
     * @param manager Non-owning pointer to the PlaylistManager.
     */
    MainPlaylistView(NcursesUI* ui, WINDOW* win, PlaylistManager* manager);
    
    /**
     * @brief Draws the view using REAL data.
     * @param focus The currently focused sub-panel.
     */
    void draw(FocusArea focus) override;
    
    /**
     * @brief Handles input for navigating the panels.
     */
    void handleInput(InputEvent event, FocusArea focus) override;

private:
    NcursesUI* ui;
    WINDOW* win;
    PlaylistManager* playlistManager; // <-- Con trỏ (không sở hữu) đến Model
    
    // --- UI State ---
    int playlistSelected; // Index trong danh sách playlist
    int trackSelected;    // Index trong danh sách bài hát
};