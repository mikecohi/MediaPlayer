#include <ncursesw/ncurses.h>
#include <vector>
#include <string>
#include <chrono>
#include <cstring>
#include <iostream>      
#include <filesystem>
#include <locale.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include <thread>
#include <atomic>
#include <unistd.h>

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/audioproperties.h>
#include <fstream>  

enum ScreenType {
    SCREEN_MAIN_CONSOLE,
    SCREEN_THIS_PC,
    SCREEN_USB,
    SCREEN_PLAYLIST,
    SCREEN_BOARD,
    SCREEN_EXIT,
    SCREEN_METADATA_DETAIL,
    SCREEN_ADD_PLAYLIST
};

// ========== AUDIO PLAYER ==========
Mix_Music* currentMusic = nullptr;

int getSongDuration(const std::string& filePath) {
    TagLib::FileRef f(filePath.c_str());
    if (!f.isNull() && f.audioProperties()) {
        return f.audioProperties()->length();  
    }   
    return 0;
}

bool initAudio() {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL could not initialize! " << SDL_GetError() << "\n";
        return false;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer could not initialize! " << Mix_GetError() << "\n";
        return false;
    }
    return true;
}

void playMusic(const std::string& path) {
    if (currentMusic != nullptr) {
        Mix_HaltMusic();
        Mix_FreeMusic(currentMusic);
        currentMusic = nullptr;
    }
    currentMusic = Mix_LoadMUS(path.c_str());
    if (!currentMusic) {
        std::cerr << "Failed to load music: " << Mix_GetError() << "\n";
        return;
    }
    Mix_PlayMusic(currentMusic, 1);
}

void closeAudio() {
    if (currentMusic != nullptr) {
        Mix_HaltMusic();
        Mix_FreeMusic(currentMusic);
    }
    Mix_CloseAudio();
    SDL_Quit();
}

// ===================================

class MainMenuView {
private:
    WINDOW *top_bar, *main_area, *bottom_bar;
    int height, width;
    const int SONG_LIST_START_Y = 2; 

    int pointed_button;
    int selected_button;

    std::vector<std::string> buttons;
    std::vector<std::string> controls;
    float progress;
    int volume;

    ScreenType currentScreen;
    std::vector<std::string> songList;
    std::string musicFolder = "test_media";
    int currentSongIndex = -1;
    bool isPaused = false;

    std::thread progressThread;
    std::atomic<bool> running;
    int songDuration = 0;
    int currentTime = 0;

    int currentPage = 0;
    const int SONGS_PER_PAGE = 20;

    int metadataSongIndex = -1;

    std::vector<std::string> playlistNames;
    int selectedPlaylistIndex = -1;
    bool viewingPlaylistSongs = false;  // Để biết đang xem bài hát trong playlist hay danh sách playlist
    std::string currentPlaylistFile;     // Lưu tên file playlist đang xem

    // Biến cho màn hình tạo playlist
    std::vector<std::string> availableSongs;
    std::vector<std::string> selectedSongs;
    std::string newPlaylistName;
    bool editingName = false;
    int scrollOffset = 0;

public:
    MainMenuView() {
        setlocale(LC_ALL, "");
        initscr();
        noecho();
        cbreak();
        curs_set(0);
        keypad(stdscr, TRUE);
        mousemask(BUTTON1_CLICKED | BUTTON3_CLICKED | BUTTON4_PRESSED | BUTTON5_PRESSED, NULL);
        currentScreen = SCREEN_MAIN_CONSOLE;
        std::string musicFolder = "test_media";    
        erase();
        clear();
        refresh();
        
        getmaxyx(stdscr, height, width);

        int top_bar_height = 3;
        int bottom_bar_height = 4;
        int main_height = height - top_bar_height - bottom_bar_height;

        top_bar = newwin(top_bar_height, width, 0, 0);
        main_area = newwin(main_height, width, top_bar_height, 0);
        bottom_bar = newwin(bottom_bar_height, width, top_bar_height + main_height, 0);

        buttons = {"Main console", "This PC", "From USB", "Playlist", "Board", "Exit"};
        controls = {"<<", "||", ">>","Vol-", "Vol+"};
        pointed_button = 0;
        selected_button = 0;
        progress = 0.0f;
        volume = 50;
        running = true;
        newPlaylistName = "New Playlist";
        progressThread = std::thread(&MainMenuView::updateProgress, this);
        drawUI();
    }

    ~MainMenuView() {
        running = false;
        if (progressThread.joinable()) progressThread.join();
        delwin(top_bar);
        delwin(main_area);
        delwin(bottom_bar);
        mousemask(0, NULL);
        endwin();
    }

    void updateProgress() {
        while (running) {
            if (currentMusic && Mix_PlayingMusic()) {
                if (!isPaused && currentTime < songDuration) {
                    currentTime++;
                    progress = (float)currentTime / (float)songDuration;
                    drawBottomBar();
                }
            }
            sleep(1);
        }
    }

    // ===== LOAD SONGS =====
    void loadSongsFromPC(const std::string& folder) {
        songList.clear();
        if (!std::filesystem::exists(folder)) return;
        for (const auto& entry : std::filesystem::directory_iterator(folder)) {
            if (entry.is_regular_file()) {
                auto path = entry.path();
                if (path.extension() == ".mp3" || path.extension() == ".wav" || path.extension() == ".ogg") {
                    songList.push_back(path.filename().string());
                }
            }
        }
    }

    void loadSongsFromUSB(const std::string& folder) {
        songList.clear();
        if (!std::filesystem::exists(folder)) return;
        for (const auto& entry : std::filesystem::directory_iterator(folder)) {
            if (entry.is_regular_file()) {
                auto path = entry.path();
                if (path.extension() == ".mp3" || path.extension() == ".wav" || path.extension() == ".ogg") {
                    songList.push_back(path.filename().string());
                }
            }
        }
    }

    void loadSongsFromPlaylist(const std::string& playlistFile) {
        songList.clear();
        std::ifstream file(playlistFile);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                if (!line.empty()) {
                    songList.push_back(line);
                }
            }
            file.close();
        }
    }

    void loadPlaylists() {
        playlistNames.clear();
        std::string folder = "./playlist";

        if (!std::filesystem::exists(folder)) {
            std::filesystem::create_directory(folder);
        }

        for (const auto &entry : std::filesystem::directory_iterator(folder)) {
            if (entry.is_directory()) {
                playlistNames.push_back(entry.path().filename().string());
            }
        }
    }

    void loadAvailableSongs() {
        availableSongs.clear();
        if (!std::filesystem::exists(musicFolder)) return;
        for (const auto& entry : std::filesystem::directory_iterator(musicFolder)) {
            if (entry.is_regular_file()) {
                auto path = entry.path();
                if (path.extension() == ".mp3" || path.extension() == ".wav" || path.extension() == ".ogg") {
                    availableSongs.push_back(path.filename().string());
                }
            }
        }
    }

    void drawAddPlaylistScreen() {
        werase(main_area);
        box(main_area, 0, 0);
        
        int main_h = getmaxy(main_area);
        int y = 1;

        // Ô nhập tên playlist
        mvwprintw(main_area, y, 2, "Playlist Name: ");
        if (editingName) {
            wattron(main_area, A_REVERSE);
        }
        mvwprintw(main_area, y, 18, "[%-30s]", newPlaylistName.c_str());
        if (editingName) {
            wattroff(main_area, A_REVERSE);
        }
        y += 2;

        mvwprintw(main_area, y++, 2, "Available Songs (Click + to add):");
        
        int listStartY = y;
        int maxVisible = main_h - y - 4;

        // Hiển thị danh sách bài hát có thể thêm
        for (int i = scrollOffset; i < (int)availableSongs.size() && (i - scrollOffset) < maxVisible; ++i) {
            bool alreadyAdded = false;
            for (const auto& s : selectedSongs) {
                if (s == availableSongs[i]) {
                    alreadyAdded = true;
                    break;
                }
            }
            
            if (alreadyAdded) {
                wattron(main_area, A_DIM);
                mvwprintw(main_area, y++, 4, "[V] %s", availableSongs[i].c_str());
                wattroff(main_area, A_DIM);
            } else {
                mvwprintw(main_area, y++, 4, "[+] %s", availableSongs[i].c_str());
            }
        }

        // Hiển thị các bài đã chọn
        y = main_h - 3;
        mvwprintw(main_area, y++, 2, "Selected: %d songs", (int)selectedSongs.size());
        
        // Nút Save và Cancel
        wattron(main_area, A_BOLD);
        mvwprintw(main_area, y, 2, "[Save]");
        mvwprintw(main_area, y, 12, "[Cancel]");
        wattroff(main_area, A_BOLD);

        wrefresh(main_area);
    }

    void drawPlaylistArea() {
        werase(main_area);
        box(main_area, 0, 0);

        int width = getmaxx(main_area);
        int y = 1;

        std::string addBtn = "[ + Add ]";
        std::string removeBtn = "[ - Remove ]";
        mvwprintw(main_area, y, 2, "%s", addBtn.c_str());
        mvwprintw(main_area, y, 2 + addBtn.size() + 4, "%s", removeBtn.c_str());

        y += 2;
        mvwprintw(main_area, y++, 2, "Playlists (Click to view songs):");

        for (size_t i = 0; i < playlistNames.size(); ++i) {
            if ((int)i == selectedPlaylistIndex) {
                wattron(main_area, A_REVERSE);
            }
            mvwprintw(main_area, y + i, 4, "%s", playlistNames[i].c_str());
            if ((int)i == selectedPlaylistIndex) {
                wattroff(main_area, A_REVERSE);
            }
        }

        wrefresh(main_area);
    }

void handlePlaylistClick(int mouseX, int mouseY) {
    int top_bar_height = 3;
    int localY = mouseY - top_bar_height;

    int btnY = 1;
    std::string addBtn = "[ + Add ]";
    std::string removeBtn = "[ - Remove ]";

    // ===== CLICK [ + Add ] =====
    if (localY == btnY && mouseX >= 2 && mouseX <= 2 + (int)addBtn.size()) {
        currentScreen = SCREEN_ADD_PLAYLIST;
        selectedSongs.clear();
        newPlaylistName = "New Playlist";
        loadAvailableSongs();
        scrollOffset = 0;
        drawAddPlaylistScreen();
        return;
    }

    // ===== CLICK [ - Remove ] =====
    if (localY == btnY && mouseX >= 2 + (int)addBtn.size() + 4 &&
        mouseX <= 2 + (int)addBtn.size() + 4 + (int)removeBtn.size()) {

        if (selectedPlaylistIndex >= 0 && selectedPlaylistIndex < (int)playlistNames.size()) {
            std::string folderToRemove = "./playlist/" + playlistNames[selectedPlaylistIndex];
            std::filesystem::remove_all(folderToRemove); // Xóa cả folder playlist
            loadPlaylists();

            if (selectedPlaylistIndex >= (int)playlistNames.size()) {
                selectedPlaylistIndex = (int)playlistNames.size() - 1;
            }

            drawPlaylistArea();
        }
        return;
    }

    // ===== CLICK VÀO TÊN PLAYLIST =====
    int listStartY = 4; // Dòng đầu tiên chứa danh sách playlist
    int clickedIndex = localY - listStartY;

    if (clickedIndex >= 0 && clickedIndex < (int)playlistNames.size()) {
        selectedPlaylistIndex = clickedIndex;

        // Lấy đường dẫn folder playlist
        std::string playlistFolder = "./playlist/" + playlistNames[selectedPlaylistIndex];

        // Gọi hàm đọc bài hát từ folder này
        loadSongsFromPlaylist(playlistFolder);

        // Chuyển sang chế độ xem danh sách bài hát
        viewingPlaylistSongs = true;
        currentPage = 0;

        drawSongListUI();
        return;
    }
}


    void handleAddPlaylistClick(int mouseX, int mouseY) {
        int top_bar_height = 3;
        int localY = mouseY - top_bar_height;
        int main_h = getmaxy(main_area);
        editingName = true;

        // Click vào ô tên playlist để chỉnh sửa
        if (localY == 1 && mouseX >= 18) {
            // Tạo một input dialog đơn giản
            werase(main_area);
            box(main_area, 0, 0);
            mvwprintw(main_area, main_h/2 - 2, 2, "Enter Playlist Name:");
            mvwprintw(main_area, main_h/2, 2, "> ");
            wrefresh(main_area);
            
            echo();
            curs_set(1);
            
            char buffer[100] = {0};
            // Copy tên hiện tại vào buffer
            strncpy(buffer, newPlaylistName.c_str(), 99);
            
            // Di chuyển con trỏ và cho phép chỉnh sửa
            mvwprintw(main_area, main_h/2, 4, "%-50s", buffer);
            wmove(main_area, main_h/2, 4);
            wrefresh(main_area);
            
            // Nhập tên mới
            char input[100] = {0};
            wgetnstr(main_area, input, 99);
            
            if (strlen(input) > 0) {
                newPlaylistName = input;
            }
            
            noecho();
            curs_set(0);
            
            // Vẽ lại màn hình
            drawAddPlaylistScreen();
            return;
        }

        // Click vào bài hát để thêm
        int listStartY = 4;
        int maxVisible = main_h - listStartY - 4;
        int clickedIndex = scrollOffset + (localY - listStartY);
        
        if (localY >= listStartY && localY < listStartY + maxVisible && 
            clickedIndex >= 0 && clickedIndex < (int)availableSongs.size()) {
            
            std::string song = availableSongs[clickedIndex];
            bool alreadyAdded = false;
            
            for (auto it = selectedSongs.begin(); it != selectedSongs.end(); ++it) {
                if (*it == song) {
                    selectedSongs.erase(it);
                    alreadyAdded = true;
                    break;
                }
            }
            
            if (!alreadyAdded) {
                selectedSongs.push_back(song);
            }
            
            drawAddPlaylistScreen();
            return;
        }

        // Click nút Save
        if (localY == main_h - 3 && mouseX >= 2 && mouseX <= 8) {
            if (selectedSongs.size() > 0) {  // Chỉ save nếu có bài hát
                savePlaylist();
                currentScreen = SCREEN_PLAYLIST;
                loadPlaylists();
                
                // Vẽ lại toàn bộ
                clear();
                refresh();
                drawTopBar();
                drawPlaylistArea();
                drawBottomBar();
            }
            return;
        }

        // Click nút Cancel
        if (localY == main_h - 3 && mouseX >= 12 && mouseX <= 20) {
            currentScreen = SCREEN_PLAYLIST;
            loadPlaylists();
            
            // Vẽ lại toàn bộ
            clear();
            refresh();
            drawTopBar();
            drawPlaylistArea();
            drawBottomBar();
            return;
        }
    }

 void savePlaylist() {
    // Tạo thư mục playlist nếu chưa có
    if (!std::filesystem::exists("./playlist")) {
        std::filesystem::create_directory("./playlist");
    }

    // Loại bỏ đuôi .txt nếu có
    std::string folderName = newPlaylistName;
    if (folderName.size() > 4 && folderName.substr(folderName.size() - 4) == ".txt") {
        folderName = folderName.substr(0, folderName.size() - 4);
    }

    // Tạo đường dẫn thư mục playlist
    std::string playlistFolderPath = "./playlist/" + folderName;
    if (!std::filesystem::exists(playlistFolderPath)) {
        std::filesystem::create_directory(playlistFolderPath);
    }

    // Copy các bài hát đã chọn vào thư mục playlist
    for (const auto &song : selectedSongs) {
        std::string sourcePath = musicFolder + "/" + song;
        std::string destPath = playlistFolderPath + "/" + song;

        try {
            std::filesystem::copy_file(sourcePath, destPath,
                std::filesystem::copy_options::overwrite_existing);
        } catch (const std::filesystem::filesystem_error &e) {
            std::cerr << "Copy failed for " << song << ": " << e.what() << "\n";
        }
    }

    // Tạo file playlist.txt để lưu danh sách tên bài hát
    std::string listFile = playlistFolderPath + "/playlist.txt";
    std::ofstream file(listFile);
    if (file.is_open()) {
        for (const auto &song : selectedSongs) {
            file << song << "\n";
        }
        file.close();
    }

    // Cập nhật lại danh sách playlist sau khi lưu xong
    loadPlaylists();
}


    // ===== Draw UI Components =====
    void drawTopBar() {
        werase(top_bar);
        box(top_bar, 0, 0);
        int spacing = width / (int)buttons.size();

        for (int i = 0; i < (int)buttons.size(); i++) {
            if (i == pointed_button)
                wattron(top_bar, A_REVERSE);
            if (i == selected_button)
                wattron(top_bar, A_UNDERLINE);

            mvwprintw(top_bar, 1, i * spacing + 2, "%s", buttons[i].c_str());

            wattroff(top_bar, A_REVERSE);
            wattroff(top_bar, A_UNDERLINE);
        }

        wrefresh(top_bar);
    }

    void drawSongListUI() {
        werase(main_area);
        box(main_area, 0, 0);

        int main_area_height = getmaxy(main_area);
        int maxVisible = main_area_height - SONG_LIST_START_Y - 1;

        int startIndex = currentPage * SONGS_PER_PAGE;
        int endIndex = std::min((int)songList.size(), startIndex + maxVisible);

        // Hiển thị tiêu đề khác nếu đang xem playlist
        if (currentScreen == SCREEN_PLAYLIST && viewingPlaylistSongs) {
            mvwprintw(main_area, 1, 1, "Playlist: %s | Page: %d/%d ", 
                      playlistNames[selectedPlaylistIndex].c_str(),
                      currentPage + 1, 
                      (int)ceil((double)songList.size() / maxVisible));
        } else {
            mvwprintw(main_area, 1, 1, "Page: %d/%d", 
                      currentPage + 1, 
                      (int)ceil((double)songList.size() / maxVisible));
        }

        int y = SONG_LIST_START_Y;
        for (int i = startIndex; i < endIndex; ++i) {
            mvwprintw(main_area, y, 4, "%2d. %s", i + 1, songList[i].c_str());
            y++;
        }

        wrefresh(main_area);
    }

    void drawMetadataDetailUI(const std::string &filePath) {
        werase(main_area);
        box(main_area, 0, 0);

        mvwprintw(main_area, 1, 2, "Metadata for: %s", filePath.c_str());

        TagLib::FileRef f(filePath.c_str());
        if (!f.isNull() && f.tag()) {
            TagLib::Tag *tag = f.tag();
            TagLib::AudioProperties *properties = f.audioProperties();

            int line = 3;
            mvwprintw(main_area, line++, 4, "Title:  %s", tag->title().toCString(true));
            mvwprintw(main_area, line++, 4, "Artist: %s", tag->artist().toCString(true));
            mvwprintw(main_area, line++, 4, "Album:  %s", tag->album().toCString(true));
            mvwprintw(main_area, line++, 4, "Year:   %d", tag->year());

            if (properties) {
                mvwprintw(main_area, line++, 4, "Length: %d sec", properties->length());
                mvwprintw(main_area, line++, 4, "Bitrate: %d kbps", properties->bitrate());
                mvwprintw(main_area, line++, 4, "Sample rate: %d Hz", properties->sampleRate());
            }
        } else {
            mvwprintw(main_area, 3, 4, "Cannot read metadata.");
        }

        mvwprintw(main_area, getmaxy(main_area)-2, 2, "Right-click again or press ESC to return.");
        wrefresh(main_area);
    }

    void drawMainArea() {
        werase(main_area);
        box(main_area, 0, 0);
        mvwprintw(main_area, 1, 2, "Selected: %s", buttons[selected_button].c_str());
        wrefresh(main_area);
    }

    void drawBottomBar() {
        werase(bottom_bar);
        box(bottom_bar, 0, 0);

        if (currentSongIndex >= 0 && currentSongIndex < (int)songList.size()) {
            mvwprintw(bottom_bar, 0, 2, "Playing: %s", songList[currentSongIndex].c_str());
        }

        const std::string label = "Progress: ";
        int left_padding = 2;
        int right_padding = 2;
        int available_for_bar = width - left_padding - right_padding - (int)label.size() - 2;
        if (available_for_bar < 8) available_for_bar = 8;

        int filled = (int)(available_for_bar * progress);
        if (filled > available_for_bar) filled = available_for_bar;
        std::string bar = "[";
        bar += std::string(filled, '#');
        bar += std::string(available_for_bar - filled, '-');
        bar += "]";

        int y_progress = 1;
        int x_progress = left_padding;
        mvwprintw(bottom_bar, y_progress, x_progress, "%s", label.c_str());
        x_progress += (int)label.size();
        mvwprintw(bottom_bar, y_progress, x_progress, "%s", bar.c_str());
      
        int minutes = currentTime / 60;
        int seconds = currentTime % 60;
        int totalMinutes = songDuration / 60;
        int totalSeconds = songDuration % 60;
        char timeBuf[32];
        snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d / %02d:%02d", minutes, seconds, totalMinutes, totalSeconds);
        mvwprintw(bottom_bar, y_progress, width - (int)strlen(timeBuf) - 2, "%s", timeBuf);

        int y_controls = 2;
        int spacing_between = 4;
        int total_ctrl_width = 0;
        for (const auto &c : controls) {
            total_ctrl_width += 2 + (int)c.size();
        }
        total_ctrl_width += spacing_between * ((int)controls.size() - 1);

        int x_start = (width - total_ctrl_width) / 2;
        if (x_start < 1) x_start = 1;

        wattron(bottom_bar, A_DIM);
        int xx = x_start;
        for (size_t i = 0; i < controls.size(); ++i) {
            mvwprintw(bottom_bar, y_controls, xx, "[%s]", controls[i].c_str());
            xx += 2 + (int)controls[i].size();
            if (i + 1 < controls.size()) xx += spacing_between;
        }
        wattroff(bottom_bar, A_DIM);

        wrefresh(bottom_bar);
    }

    void drawUI() {
        drawTopBar();
        drawMainArea();
        drawBottomBar();
    }

    // ===== Handle Input =====
    void handleInput() {
        MEVENT event;
        int ch;
        while (true) {
            ch = getch();
            switch (ch) {
                case KEY_LEFT:
                    pointed_button = (pointed_button - 1 + buttons.size()) % buttons.size();
                    drawTopBar();
                    break;

                case KEY_RIGHT:
                    pointed_button = (pointed_button + 1) % buttons.size();
                    drawTopBar();
                    break;

                case '\n':
                    selected_button = pointed_button;
                    if (buttons[selected_button] == "Exit") return;
                    selectButton(selected_button);
                    drawTopBar();
                    break;

                case KEY_NPAGE:
                    if (currentScreen == SCREEN_ADD_PLAYLIST) {
                        int main_h = getmaxy(main_area);
                        int maxVisible = main_h - 4 - 4;
                        if (scrollOffset + maxVisible < (int)availableSongs.size()) {
                            scrollOffset++;
                            drawAddPlaylistScreen();
                        }
                    } else if ((currentPage + 1) * SONGS_PER_PAGE < (int)songList.size()) {
                        currentPage++;
                        drawSongListUI();
                    }
                    break;

                case KEY_PPAGE:
                    if (currentScreen == SCREEN_ADD_PLAYLIST) {
                        if (scrollOffset > 0) {
                            scrollOffset--;
                            drawAddPlaylistScreen();
                        }
                    } else if (currentPage > 0) {
                        currentPage--;
                        drawSongListUI();
                    }
                    break;

                case KEY_MOUSE:
                    if (getmouse(&event) == OK) {
                        if (event.bstate & BUTTON1_CLICKED) {
                            handleMouseClick(event.x, event.y, event);
                        } else if (event.bstate & BUTTON3_CLICKED) {
                            handleMouseClick(event.x, event.y, event);
                        } else if (event.bstate & BUTTON4_PRESSED) {
                            if (currentScreen == SCREEN_ADD_PLAYLIST) {
                                if (scrollOffset > 0) {
                                    scrollOffset--;
                                    drawAddPlaylistScreen();
                                }
                            } else if (currentPage > 0) {
                                currentPage--;
                                drawSongListUI();
                            }
                        } else if (event.bstate & BUTTON5_PRESSED) {
                            if (currentScreen == SCREEN_ADD_PLAYLIST) {
                                int main_h = getmaxy(main_area);
                                int maxVisible = main_h - 4 - 4;
                                if (scrollOffset + maxVisible < (int)availableSongs.size()) {
                                    scrollOffset++;
                                    drawAddPlaylistScreen();
                                }
                            } else if ((currentPage + 1) * SONGS_PER_PAGE < (int)songList.size()) {
                                currentPage++;
                                drawSongListUI();
                            }
                        }
                    }
                    break;

                case 27:
                    if (currentScreen == SCREEN_METADATA_DETAIL) {
                        if (viewingPlaylistSongs) {
                            currentScreen = SCREEN_PLAYLIST;
                            drawSongListUI();
                        } else {
                            currentScreen = SCREEN_THIS_PC;
                            drawSongListUI();
                        }
                    } else if (currentScreen == SCREEN_ADD_PLAYLIST) {
                        currentScreen = SCREEN_PLAYLIST;
                        viewingPlaylistSongs = false;
                        loadPlaylists();
                        drawPlaylistArea();
                    } else if (currentScreen == SCREEN_PLAYLIST && viewingPlaylistSongs) {
                        // Quay lại danh sách playlist
                        viewingPlaylistSongs = false;
                        loadPlaylists();
                        drawPlaylistArea();
                    }
                    break;
            }
        }
    }

    // ===== Mouse Click Handling =====
    void handleMouseClick(int x, int y, MEVENT event) {
        int top_bar_height = 3;

        if (y < top_bar_height) {
            int spacing = width / buttons.size();
            int clicked_index = x / spacing;
            if (clicked_index >= 0 && clicked_index < (int)buttons.size()) {
                pointed_button = clicked_index;
                selected_button = clicked_index;
                if (buttons[clicked_index] == "Exit") {
                    endwin();
                    exit(0);
                }
                selectButton(clicked_index);
                drawTopBar();
            }
        }

        if (currentScreen == SCREEN_ADD_PLAYLIST) {
            handleAddPlaylistClick(x, y);
            return;
        }

        if (currentScreen == SCREEN_PLAYLIST) {
            handlePlaylistClick(x, y);
            return;
        }

        if ((currentScreen == SCREEN_THIS_PC || currentScreen == SCREEN_USB) || 
            (currentScreen == SCREEN_PLAYLIST && viewingPlaylistSongs)) {
            int bottom_bar_height = 4;
            int main_area_height = height - top_bar_height - bottom_bar_height;

            if (y >= top_bar_height + SONG_LIST_START_Y && y < top_bar_height + main_area_height) {
                int indexInPage = y - (top_bar_height + SONG_LIST_START_Y);
                int index = currentPage * SONGS_PER_PAGE + indexInPage;

                int startIndex = currentPage * SONGS_PER_PAGE;
                int endIndex = std::min((int)songList.size(), startIndex + SONGS_PER_PAGE);
                int numSongsInPage = endIndex - startIndex;

                if (indexInPage >= 0 && indexInPage < numSongsInPage) {
                    std::string folderPath;
                    if (currentScreen == SCREEN_THIS_PC) {
                        folderPath = musicFolder;
                    } else if (currentScreen == SCREEN_USB) {
                        folderPath = "/music/usb";
                    } else if (viewingPlaylistSongs) {
                        folderPath = musicFolder;  // Bài hát trong playlist lưu tên file, cần đường dẫn đầy đủ
                    }
                    std::string fullPath = folderPath + "/" + songList[index];

                    if (event.bstate & BUTTON1_CLICKED) {
                        currentSongIndex = index;
                        isPaused = false;
                        currentTime = 0;
                        progress = 0;
                        songDuration = getSongDuration(fullPath);
                        playMusic(fullPath);
                        drawBottomBar();
                    } else if (event.bstate & BUTTON3_CLICKED) {
                        metadataSongIndex = index;
                        currentScreen = SCREEN_METADATA_DETAIL;
                        drawMetadataDetailUI(fullPath);
                    }
                }
            }
        }

        int bottom_start_y = height - 4;
        if (y >= bottom_start_y && y < height) {
            handleControlClick(x, y);
        }
    }

    void handleControlClick(int x, int y) {
        int spacing_between = 4;
        int total_ctrl_width = 0;
        for (const auto &c : controls) {
            total_ctrl_width += 2 + (int)c.size();
        }
        total_ctrl_width += spacing_between * ((int)controls.size() - 1);

        int x_start = (width - total_ctrl_width) / 2;
        int xx = x_start;

        for (size_t i = 0; i < controls.size(); ++i) {
            int ctrl_width = 2 + (int)controls[i].size();
            if (x >= xx && x <= xx + ctrl_width) {
                executeControl(i);
                break;
            }
            xx += ctrl_width + spacing_between;
        }
    }

    void executeControl(int ctrlIndex) {
        if (ctrlIndex == 0) {
            if (currentSongIndex > 0) {
                currentSongIndex--;
                std::string path = musicFolder + "/" + songList[currentSongIndex];
                songDuration = getSongDuration(path);
                currentTime = 0;
                progress = 0;
                playMusic(path);
                isPaused = false;
                controls[1] = "||";
                drawBottomBar();
            }
        }
        else if (ctrlIndex == 1) {
            if (isPaused) {
                Mix_ResumeMusic();
                isPaused = false;
                controls[1] = "||";
                drawBottomBar();
            } else {
                Mix_PauseMusic();
                isPaused = true;
                controls[1] = "|>";
                drawBottomBar();            
            }
        }
        else if (ctrlIndex == 2) {
            if (currentSongIndex >= 0 && currentSongIndex < (int)songList.size() - 1) {
                currentSongIndex++;
                std::string path = musicFolder + "/" + songList[currentSongIndex];
                songDuration = getSongDuration(path);
                currentTime = 0;
                progress = 0;
                playMusic(path);
                isPaused = false;
                controls[1] = "||";
                drawBottomBar();
            }
        }
        else if (ctrlIndex == 3) {
            int vol = Mix_VolumeMusic(-1);
            vol = std::max(0, vol - 10);
            Mix_VolumeMusic(vol);
            volume = (int)(vol * 100.0 / 128);
        }
        else if (ctrlIndex == 4) {
            int vol = Mix_VolumeMusic(-1);
            vol = std::min(128, vol + 10);
            Mix_VolumeMusic(vol);
            volume = (int)(vol * 100.0 / 128);
        }
        drawBottomBar();
    }

    // ===== Actions =====
    void selectButton(int index) {
        std::string name = buttons[index];

        if (name == "Main console") {
            currentScreen = SCREEN_MAIN_CONSOLE;
            showMessage("Home page");
        } 
        else if (name == "This PC") {
            currentScreen = SCREEN_THIS_PC;
            loadSongsFromPC(musicFolder);
            drawSongListUI();
        } 
        else if (name == "From USB") {
            currentScreen = SCREEN_USB;
            loadSongsFromUSB("/music/usb");
            drawSongListUI();
        } 
        else if (name == "Playlist") {
            currentScreen = SCREEN_PLAYLIST;
            viewingPlaylistSongs = false;
            loadPlaylists();
            drawPlaylistArea();
        } 
        else if (name == "Board") {
            currentScreen = SCREEN_BOARD;
            showMessage("Board");
        } 
        else if (name == "Exit") {
            currentScreen = SCREEN_EXIT;
            endwin();
            exit(0);
        }
    }

    void showMessage(const std::string &msg) {
        werase(main_area);
        box(main_area, 0, 0);
        mvwprintw(main_area, 1, 2, "%s", msg.c_str());
        wrefresh(main_area);
    }
};

// ===== Main =====
int main() {
    setlocale(LC_ALL, "");
    if (!initAudio()) {
        std::cerr << "Audio init failed\n";
        return 1;
    }
    MainMenuView app;
    app.handleInput();
    return 0;
}