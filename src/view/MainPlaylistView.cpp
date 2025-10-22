#include "view/MainPlaylistView.h"
#include "model/Playlist.h"
#include "model/MediaFile.h"
#include <algorithm>
#include <string>
#include <tuple>
#include <vector> // Needed for tracks vector

MainPlaylistView::MainPlaylistView(NcursesUI* ui, WINDOW* win, PlaylistManager* manager)
    : ui(ui), win(win), playlistManager(manager),
      playlistSelected(0), trackSelected(0)
{
    // Initialize button positions
    createBtnY = createBtnX = createBtnW = 0;
    deleteBtnY = deleteBtnX = deleteBtnW = 0;
    playBtnY = playBtnX = playBtnW = 0;
    removeBtnY = removeBtnX = removeBtnW = 0;
}

void MainPlaylistView::draw(FocusArea focus) {
    werase(win);
    box(win, 0, 0);

    int width, height;
    getmaxyx(win, height, width);
    int listWidth = width / 2;

    if (!playlistManager) { // Safety check
        mvwprintw(win, 2, 3, "Error: PlaylistManager is null!");
        wnoutrefresh(win);
        return;
    }

    std::vector<Playlist*> playlists = playlistManager->getAllPlaylists();

    // --- Draw playlist list panel ---
    mvwprintw(win, 2, 3, "Playlists (%d)", static_cast<int>(playlists.size()));
    int maxPlaylistsToShow = height - 6; // Reserve lines
    if (maxPlaylistsToShow < 0) maxPlaylistsToShow = 0;

    for (size_t i = 0; i < playlists.size() && (int)i < maxPlaylistsToShow; ++i) {
        if (focus == FocusArea::MAIN_LIST && (int)i == playlistSelected) {
             wattron(win, A_REVERSE | A_BOLD);
        }
        std::string name = playlists[i]->getName();
        std::string count = "(" + std::to_string(playlists[i]->getTracks().size()) + ")";
        std::string entry = name + " " + count;
        mvwprintw(win, 4 + i, 3, "%.*s", listWidth - 5, entry.c_str());
        wattroff(win, A_REVERSE | A_BOLD);
    }

    // --- Draw track list panel ---
    mvwprintw(win, 2, listWidth + 2, "Tracks in Playlist");
    std::vector<MediaFile*> tracks; // Define tracks vector here
    Playlist* currentSelectedPlaylist = nullptr;
    int maxTracksToShow = height - 6; // Reserve lines
     if (maxTracksToShow < 0) maxTracksToShow = 0;


    if (playlistSelected >= 0 && static_cast<size_t>(playlistSelected) < playlists.size()) {
        currentSelectedPlaylist = playlists[playlistSelected];
        tracks = currentSelectedPlaylist->getTracks(); // Assign tracks

        if (tracks.empty()) {
            mvwprintw(win, 4, listWidth + 2, "(No tracks)");
        } else {
            for (size_t i = 0; i < tracks.size() && (int)i < maxTracksToShow; ++i) {
                if (focus == FocusArea::MAIN_DETAIL && (int)i == trackSelected) {
                    wattron(win, A_REVERSE | A_BOLD);
                }
                mvwprintw(win, 4 + i, listWidth + 2, "%.*s", width - listWidth - 4, tracks[i]->getFileName().c_str());
                wattroff(win, A_REVERSE | A_BOLD);
            }
        }
    } else if (!playlists.empty()) {
        mvwprintw(win, 4, listWidth + 2, "(Select a playlist)");
    } else {
        mvwprintw(win, 4, listWidth + 2, "(No playlists created)");
    }

    // --- Draw Buttons ---
    // Playlist buttons
    createBtnY = height - 3; // Position near bottom
    createBtnX = 3;
    std::string createLabel = "[Create]"; createBtnW = createLabel.length();
    mvwprintw(win, createBtnY, createBtnX, "%s", createLabel.c_str());

    deleteBtnX = createBtnX + createBtnW + 2;
    std::string deleteLabel = "[Delete]"; deleteBtnW = deleteLabel.length();
    mvwprintw(win, createBtnY, deleteBtnX, "%s", deleteLabel.c_str());

    playBtnX = deleteBtnX + deleteBtnW + 2;
    std::string playLabel = "[Play]"; playBtnW = playLabel.length();
    mvwprintw(win, createBtnY, playBtnX, "%s", playLabel.c_str());

    // Track button
    removeBtnY = height - 3; // Position near bottom
    removeBtnX = listWidth + 2;
    std::string removeLabel = "[Remove Song]"; removeBtnW = removeLabel.length();
    mvwprintw(win, removeBtnY, removeBtnX, "%s", removeLabel.c_str());

    // Highlight based on focus (optional, can be complex)
    // Example: Highlight first button if focus is on the corresponding panel
    if (focus == FocusArea::MAIN_LIST) {
        wattron(win, A_BOLD);
        mvwprintw(win, createBtnY, createBtnX, "%s", createLabel.c_str());
        wattroff(win, A_BOLD);
    }
     if (focus == FocusArea::MAIN_DETAIL && !tracks.empty()) { // Only highlight if tracks exist
        wattron(win, A_BOLD);
        mvwprintw(win, removeBtnY, removeBtnX, "%s", removeLabel.c_str());
        wattroff(win, A_BOLD);
    }

    wnoutrefresh(win);
}

// Update return type to MainAreaAction
MainAreaAction MainPlaylistView::handleInput(InputEvent event, FocusArea focus) {
     if (!playlistManager) return MainAreaAction::NONE;

    std::vector<Playlist*> playlists = playlistManager->getAllPlaylists();
    int playlistCount = playlists.size();
    int trackCount = 0;
    if (playlistSelected >= 0 && static_cast<size_t>(playlistSelected) < playlists.size()) {
         trackCount = playlists[playlistSelected]->getTracks().size();
    }


    if (focus == FocusArea::MAIN_LIST) {
        int oldSelected = playlistSelected;
        if (event.key == KEY_DOWN && playlistCount > 0) playlistSelected = std::min(playlistSelected + 1, playlistCount - 1);
        if (event.key == KEY_UP) playlistSelected = std::max(0, playlistSelected - 1);
        if (playlistSelected != oldSelected) trackSelected = 0; // Reset track selection

        // Handle Enter key on buttons (simplified: Enter activates first button)
        if (event.key == 10) return MainAreaAction::CREATE_PLAYLIST; // Assume Enter hits Create
        // Handle 'c' key for create
        if (event.key == 'c') return MainAreaAction::CREATE_PLAYLIST;
        // Handle 'd' key for delete (requires selection check)
        if (event.key == 'd' && playlistCount > 0 && playlistSelected >= 0) return MainAreaAction::DELETE_PLAYLIST;

    } else if (focus == FocusArea::MAIN_DETAIL) {
        if (trackCount > 0) {
            if (event.key == KEY_DOWN) trackSelected = std::min(trackSelected + 1, trackCount - 1);
            if (event.key == KEY_UP) trackSelected = std::max(0, trackSelected - 1);
        }
        // Handle Enter key on button (simplified: Enter activates Remove Song)
        if (event.key == 10 && trackCount > 0 && trackSelected >= 0) return MainAreaAction::REMOVE_TRACK_FROM_PLAYLIST;
    }

    return MainAreaAction::NONE;
}

// Update return type to MainAreaAction
MainAreaAction MainPlaylistView::handleMouse(int localY, int localX) {
     if (!playlistManager) return MainAreaAction::NONE;

    int width; getmaxyx(win, std::ignore, width); int listWidth = width / 2;
    int listStartY = 4;
    int clickedIndex = localY - listStartY;

    // Check button clicks first
    if (localY == createBtnY) { // Button row Y coordinate
        if (localX >= createBtnX && localX < createBtnX + createBtnW) return MainAreaAction::CREATE_PLAYLIST;
        if (localX >= deleteBtnX && localX < deleteBtnX + deleteBtnW) return MainAreaAction::DELETE_PLAYLIST;
        if (localX >= playBtnX && localX < playBtnX + playBtnW) return MainAreaAction::PLAY_PLAYLIST;
    }
    if (localY == removeBtnY) { // Button row Y coordinate (can be same as createBtnY)
         if (localX >= removeBtnX && localX < removeBtnX + removeBtnW) return MainAreaAction::REMOVE_TRACK_FROM_PLAYLIST;
    }


    // Handle list clicks if buttons weren't clicked
    std::vector<Playlist*> playlists = playlistManager->getAllPlaylists();
    if (playlists.empty()) return MainAreaAction::NONE;

    if (localX < listWidth) { // Clicked on playlist list
        if (clickedIndex >= 0 && static_cast<size_t>(clickedIndex) < playlists.size()) {
            if (playlistSelected != clickedIndex) {
                 playlistSelected = clickedIndex;
                 trackSelected = 0;
            }
        }
    } else { // Clicked on track list
        if (playlistSelected >= 0 && static_cast<size_t>(playlistSelected) < playlists.size()) {
            int trackCount = playlists[playlistSelected]->getTracks().size();
            if (clickedIndex >= 0 && clickedIndex < trackCount) {
                trackSelected = clickedIndex;
            }
        }
    }
    return MainAreaAction::NONE; // Click was on list item, not button
}


MediaFile* MainPlaylistView::getSelectedTrack() const {
     if (!playlistManager) return nullptr;
    std::vector<Playlist*> playlists = playlistManager->getAllPlaylists();
    if (playlistSelected < 0 || static_cast<size_t>(playlistSelected) >= playlists.size()) return nullptr;
    Playlist* selectedPlaylist = playlists[playlistSelected];
    const std::vector<MediaFile*>& tracks = selectedPlaylist->getTracks();
    if (trackSelected < 0 || static_cast<size_t>(trackSelected) >= tracks.size()) return nullptr;
    return tracks[trackSelected];
}

Playlist* MainPlaylistView::getSelectedPlaylist() const {
    if (!playlistManager) return nullptr;
    std::vector<Playlist*> playlists = playlistManager->getAllPlaylists();
    if (playlistSelected >= 0 && static_cast<size_t>(playlistSelected) < playlists.size()) {
        return playlists[playlistSelected];
    }
    return nullptr;
}

int MainPlaylistView::getSelectedPlaylistIndex() const {
     if (!playlistManager) return -1;
     std::vector<Playlist*> playlists = playlistManager->getAllPlaylists();
     if (playlistSelected >= 0 && static_cast<size_t>(playlistSelected) < playlists.size()) {
        return playlistSelected;
     }
     return -1;
}