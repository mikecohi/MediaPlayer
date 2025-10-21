#include "view/MainPlaylistView.h"
#include "model/Playlist.h"   // Cần để lấy tên và danh sách bài hát
#include "model/MediaFile.h"  // Cần để lấy tên file
#include <algorithm> // For std::min/max
#include <string>    // For std::to_string
#include <tuple>     // For std::ignore

MainPlaylistView::MainPlaylistView(NcursesUI* ui, WINDOW* win, PlaylistManager* manager)
    : ui(ui), win(win), playlistManager(manager), 
      playlistSelected(0), trackSelected(0)
{
    // Constructor. Không cần làm gì thêm,
    // vì chúng ta sẽ lấy dữ liệu mới mỗi khi draw().
}

void MainPlaylistView::draw(FocusArea focus) {
    wclear(win); // Xóa nội dung cũ
    box(win, 0, 0); // Vẽ lại viền

    int width;
    getmaxyx(win, std::ignore, width);
    int listWidth = width / 2;

    // --- 1. LẤY DỮ LIỆU THẬT TỪ MODEL ---
    std::vector<Playlist*> playlists = playlistManager->getAllPlaylists();

    // --- 2. VẼ PANEL DANH SÁCH PLAYLIST (Bên trái) ---
mvwprintw(win, 2, 3, "Playlists (%d)", static_cast<int>(playlists.size()));

    for (size_t i = 0; i < playlists.size(); ++i) {
        if (focus == FocusArea::MAIN_LIST && (int)i == playlistSelected) {
             wattron(win, A_REVERSE | A_BOLD);
        }
        
        // Tạo chuỗi tên (ví dụ: "Chill Vibes (12)")
        std::string name = playlists[i]->getName();
        std::string count = "(" + std::to_string(playlists[i]->getTracks().size()) + ")";
        std::string entry = name + " " + count;
        
        mvwprintw(win, 4 + i, 3, "%.*s", listWidth - 5, entry.c_str());
        wattroff(win, A_REVERSE | A_BOLD);
    }
    
    // --- 3. VẼ PANEL CHI TIẾT BÀI HÁT (Bên phải) ---
    mvwprintw(win, 2, listWidth + 2, "Tracks in Playlist");

    if (playlistSelected >= 0 && (size_t)playlistSelected < playlists.size()) {
        // Lấy playlist đang được chọn
        Playlist* selectedPlaylist = playlists[playlistSelected];
        std::vector<MediaFile*> tracks = selectedPlaylist->getTracks();

        if (tracks.empty()) {
            mvwprintw(win, 4, listWidth + 2, "(No tracks)");
        } else {
            // Vẽ danh sách bài hát
            for (size_t i = 0; i < tracks.size(); ++i) {
                if (focus == FocusArea::MAIN_DETAIL && (int)i == trackSelected) {
                    wattron(win, A_REVERSE | A_BOLD);
                }
                mvwprintw(win, 4 + i, listWidth + 2, "%s", tracks[i]->getFileName().c_str());
                wattroff(win, A_REVERSE | A_BOLD);
            }
        }
    } else if (!playlists.empty()) {
        mvwprintw(win, 4, listWidth + 2, "(Invalid selection)");
    } else {
        mvwprintw(win, 4, listWidth + 2, "(No playlists created)");
    }

    wrefresh(win); // Đẩy tất cả thay đổi ra màn hình
}

void MainPlaylistView::handleInput(InputEvent event, FocusArea focus) {
    // Lấy dữ liệu mới nhất để kiểm tra giới hạn (bounds)
    std::vector<Playlist*> playlists = playlistManager->getAllPlaylists();
    
    if (focus == FocusArea::MAIN_LIST && !playlists.empty()) {
        // Xử lý input cho danh sách playlist
        if (event.key == KEY_DOWN) {
            playlistSelected = std::min(playlistSelected + 1, (int)playlists.size() - 1);
            trackSelected = 0; // Reset lựa chọn bài hát khi đổi playlist
        }
        if (event.key == KEY_UP) {
            playlistSelected = std::max(0, playlistSelected - 1);
            trackSelected = 0; // Reset
        }

    } else if (focus == FocusArea::MAIN_DETAIL && !playlists.empty() && 
               (size_t)playlistSelected < playlists.size()) {
        
        // Xử lý input cho danh sách bài hát
        int trackCount = playlists[playlistSelected]->getTracks().size();
        if (trackCount > 0) {
            if (event.key == KEY_DOWN) {
                trackSelected = std::min(trackSelected + 1, trackCount - 1);
            }
            if (event.key == KEY_UP) {
                trackSelected = std::max(0, trackSelected - 1);
            }
        }
    }
}