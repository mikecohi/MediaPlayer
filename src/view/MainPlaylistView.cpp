#include "view/MainPlaylistView.h"
#include "model/Playlist.h"
#include "model/MediaFile.h"
#include <algorithm>
#include <string>
#include <tuple>

MainPlaylistView::MainPlaylistView(NcursesUI* ui, WINDOW* win, PlaylistManager* manager)
    : ui(ui), win(win), playlistManager(manager),
      playlistSelected(0), trackSelected(0)
{}

void MainPlaylistView::draw(FocusArea focus) {
    werase(win);
    box(win, 0, 0);

    int width;
    getmaxyx(win, std::ignore, width);
    int listWidth = width / 2;

    std::vector<Playlist*> playlists = playlistManager->getAllPlaylists();

    // Draw playlist list panel
    mvwprintw(win, 2, 3, "Playlists (%d)", static_cast<int>(playlists.size()));
    for (size_t i = 0; i < playlists.size(); ++i) {
        if (focus == FocusArea::MAIN_LIST && (int)i == playlistSelected) {
             wattron(win, A_REVERSE | A_BOLD);
        }
        std::string name = playlists[i]->getName();
        std::string count = "(" + std::to_string(playlists[i]->getTracks().size()) + ")";
        std::string entry = name + " " + count;
        mvwprintw(win, 4 + i, 3, "%.*s", listWidth - 5, entry.c_str());
        wattroff(win, A_REVERSE | A_BOLD);
    }

    // Draw track list panel
    mvwprintw(win, 2, listWidth + 2, "Tracks in Playlist");
    std::vector<MediaFile*> tracks;
    Playlist* currentSelectedPlaylist = nullptr; // Keep track of the selected playlist
    if (playlistSelected >= 0 && static_cast<size_t>(playlistSelected) < playlists.size()) {
        currentSelectedPlaylist = playlists[playlistSelected];
        tracks = currentSelectedPlaylist->getTracks();

        if (tracks.empty()) {
            mvwprintw(win, 4, listWidth + 2, "(No tracks)");
        } else {
            for (size_t i = 0; i < tracks.size(); ++i) {
                if (focus == FocusArea::MAIN_DETAIL && (int)i == trackSelected) {
                    wattron(win, A_REVERSE | A_BOLD);
                }
                mvwprintw(win, 4 + i, listWidth + 2, "%.*s", width - listWidth - 4, tracks[i]->getFileName().c_str());
                wattroff(win, A_REVERSE | A_BOLD);
            }
        }
    } else if (!playlists.empty()) {
        mvwprintw(win, 4, listWidth + 2, "(Invalid playlist selection)");
    } else {
        mvwprintw(win, 4, listWidth + 2, "(No playlists created)");
    }
     // TODO: Add buttons [Create] [Delete] [Play] and [Remove Song] visually

    wnoutrefresh(win);
}

void MainPlaylistView::handleInput(InputEvent event, FocusArea focus) {
    std::vector<Playlist*> playlists = playlistManager->getAllPlaylists();
    if (focus == FocusArea::MAIN_LIST && !playlists.empty()) {
        int oldSelected = playlistSelected;
        if (event.key == KEY_DOWN) playlistSelected = std::min(playlistSelected + 1, (int)playlists.size() - 1);
        if (event.key == KEY_UP) playlistSelected = std::max(0, playlistSelected - 1);
        if (playlistSelected != oldSelected) trackSelected = 0; // Reset track selection on playlist change

    } else if (focus == FocusArea::MAIN_DETAIL && !playlists.empty() &&
               static_cast<size_t>(playlistSelected) < playlists.size()) {
        int trackCount = playlists[playlistSelected]->getTracks().size();
        if (trackCount > 0) {
            if (event.key == KEY_DOWN) trackSelected = std::min(trackSelected + 1, trackCount - 1);
            if (event.key == KEY_UP) trackSelected = std::max(0, trackSelected - 1);
        }
    }
     // TODO: Handle Enter key on playlist buttons ([Create] etc.)
     // TODO: Handle Enter key on track buttons ([Remove Song])
}

void MainPlaylistView::handleMouse(int localY, int localX) {
    int width;
    getmaxyx(win, std::ignore, width);
    int listWidth = width / 2;
    int listStartY = 4; // Both lists start at Y=4

    std::vector<Playlist*> playlists = playlistManager->getAllPlaylists();
    if (playlists.empty()) return;

    int clickedIndex = localY - listStartY;

    if (localX < listWidth) { // Clicked on playlist list (left panel)
        if (clickedIndex >= 0 && static_cast<size_t>(clickedIndex) < playlists.size()) {
            if (playlistSelected != clickedIndex) { // Only reset if selection changed
                 playlistSelected = clickedIndex;
                 trackSelected = 0; // Reset track selection
            }
        }
    } else { // Clicked on track list (right panel)
        if (playlistSelected >= 0 && static_cast<size_t>(playlistSelected) < playlists.size()) {
            int trackCount = playlists[playlistSelected]->getTracks().size();
            if (clickedIndex >= 0 && clickedIndex < trackCount) {
                trackSelected = clickedIndex;
            }
        }
    }
     // TODO: Handle clicks on buttons ([Create], [Delete], [Remove Song])
}


MediaFile* MainPlaylistView::getSelectedTrack() const {
    std::vector<Playlist*> playlists = playlistManager->getAllPlaylists();
    if (playlistSelected < 0 || static_cast<size_t>(playlistSelected) >= playlists.size()) return nullptr;
    Playlist* selectedPlaylist = playlists[playlistSelected];
    const std::vector<MediaFile*>& tracks = selectedPlaylist->getTracks();
    if (trackSelected < 0 || static_cast<size_t>(trackSelected) >= tracks.size()) return nullptr;
    return tracks[trackSelected];
}