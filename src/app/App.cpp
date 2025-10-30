#include "app/App.h"
#include "utils/NcursesUI.h"
#include "utils/FileUtils.h"
#include "view/UIManager.h"
#include "controller/AppController.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <unistd.h>    // for getlogin()
#include <cstdlib>     // for getenv()

#include "model/MediaManager.h"
#include "model/PlaylistManager.h"

namespace fs = std::filesystem;

App::App() {}
App::~App() {
    std::cout << "App: Cleanup complete." << std::endl;
}

/**
 * @brief Lấy thư mục Music của người dùng để chứa media và playlist
 * @return fs::path dạng /home/<user>/Music/MediaPlayer/
 */
static fs::path getUserMusicRoot() {
    const char* home = getenv("HOME");
    if (!home) home = getlogin();
    fs::path root = fs::path(home) / "Music" / "MediaPlayer";

    // Tạo thư mục nếu chưa có
    fs::create_directories(root / "test_media");
    fs::create_directories(root / "playlist");

    return root;
}

bool App::init() {
    ui = std::make_unique<NcursesUI>();
    if (!ui->initScreen()) {
        std::cerr << "Failed to initialize NcursesUI!" << std::endl;
        return false;
    }

    appController = std::make_unique<AppController>();
    if (!appController->init()) {
        std::cerr << "Failed to initialize AppController!" << std::endl;
        return false;
    }

    uiManager = std::make_unique<UIManager>(ui.get(), appController.get());
    if (!uiManager->init()) {
        std::cerr << "Failed to initialize UIManager!" << std::endl;
        return false;
    }

    // 🔹 Lấy đường dẫn thư mục media/playlist của người dùng
    fs::path userRoot = getUserMusicRoot();
    fs::path mediaPath = userRoot / "test_media";
    fs::path playlistPath = userRoot / "playlist" / "playlists.json";

    // --- 1️⃣ LOAD MEDIA NGƯỜI DÙNG ---
    std::cout << "App: Loading user media from " << mediaPath << " ..." << std::endl;
    appController->getMediaManager()->loadFromDirectory(mediaPath.string());

    // --- 2️⃣ LOAD USB MEDIA (NẾU CÓ) ---
    std::cout << "App: Checking for USB media..." << std::endl;
    if (appController->loadUSBLibrary()) {
        int usbCount = appController->getUSBMediaManager()->getTotalFileCount();
        std::cout << "App: USB media loaded successfully. Found " << usbCount << " files." << std::endl;
    } else {
        std::cout << "App: No USB media detected or failed to load." << std::endl;
    }

    // --- 3️⃣ LOAD PLAYLIST ---
    if (appController && appController->getPlaylistManager()) {
        std::cout << "App: Loading playlists from " << playlistPath << "..." << std::endl;
        appController->getPlaylistManager()->loadFromFile(playlistPath.string());
    }

    return true;
}

void App::run() {
    if (uiManager) uiManager->run();
}
