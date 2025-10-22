#pragma once
#include "view/IMainAreaView.h"
#include <vector>
#include <string>
#include "model/PlaylistManager.h"
#include "model/Playlist.h"
#include "model/MediaFile.h"

class MainPlaylistView : public IMainAreaView {
public:
    MainPlaylistView(NcursesUI* ui, WINDOW* win, PlaylistManager* manager);
    void draw(FocusArea focus) override;
    
    MainAreaAction handleInput(InputEvent event, FocusArea focus) override;
    MainAreaAction handleMouse(int localY, int localX) override;
    MediaFile* getSelectedTrack() const;

    /**
     * @brief Gets the currently selected playlist object.
     * @return Non-owning pointer to the selected Playlist, or nullptr.
     */
    Playlist* getSelectedPlaylist() const;

    /**
     * @brief Gets the index of the currently selected playlist.
     * @return The index, or -1 if invalid.
     */
    int getSelectedPlaylistIndex() const; 
private:
    NcursesUI* ui;
    WINDOW* win;
    PlaylistManager* playlistManager;
    int playlistSelected;
    int trackSelected;

    // Store button locations
    int createBtnY, createBtnX, createBtnW;
    int deleteBtnY, deleteBtnX, deleteBtnW;
    int playBtnY, playBtnX, playBtnW;
    int removeBtnY, removeBtnX, removeBtnW;

    // --- Pagination for Playlists ---
    int playlistPage;
    int totalPlaylistPages;
    int playlistsPerPage;

    // --- Button Coordinates ---
    int prevBtnY, prevBtnX, prevBtnW;
    int nextBtnY, nextBtnX, nextBtnW;
};