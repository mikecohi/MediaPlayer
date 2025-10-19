#include "PlaylistDetailView.h"
#include "../model/MediaFile.h"

PlaylistDetailView::PlaylistDetailView(Playlist* p, PlaylistController* c)
    : playlist(p), controller(c) {}

void PlaylistDetailView::show() {
    if (!playlist) {
        std::cout << "No playlist selected.\n";
        return;
    }
    displayDetails();
    handleUserInput();
}

void PlaylistDetailView::displayDetails() {
    std::cout << "\n===== PLAYLIST: " << playlist->getName() << " =====" << std::endl;
    const std::vector<MediaFile>& tracks = playlist->getTracks();

    if (tracks.empty()) {
        std::cout << "(No tracks)" << std::endl;
    } else {
        for (size_t i = 0; i < tracks.size(); ++i) {
            std::cout << i + 1 << ". " << tracks[i].getFileName() << std::endl;
        }
    }

    std::cout << "---------------------------------" << std::endl;
    std::cout << "[a] Add  [r] Remove  [p] Play  [b] Back" << std::endl;
}

void PlaylistDetailView::handleUserInput() {
    char choice;
    std::cout << "Choose: ";
    std::cin >> choice;
    std::cin.ignore(); // tránh nuốt dòng

    switch (choice) {
        case 'a': addTrack(); break;
        case 'r': removeTrack(); break;
        case 'p': play(); break;
        case 'b': return;
        default: std::cout << "Invalid choice.\n"; break;
    }

    // Sau khi thao tác, hiển thị lại để cập nhật
    show();
}

void PlaylistDetailView::addTrack() {
    if (!controller || !playlist) return;

    std::string fileName;
    std::cout << "Enter track name: ";
    std::getline(std::cin, fileName);

    MediaFile file(fileName, "mock_path/" + fileName);
    controller->addTrackToPlaylist(playlist->getName(), file);
    std::cout << "✅ Added track: " << fileName << std::endl;
}

void PlaylistDetailView::removeTrack() {
    if (!controller || !playlist) return;

    std::string fileName;
    std::cout << "Enter track name to remove: ";
    std::getline(std::cin, fileName);

    controller->removeTrackFromPlaylist(playlist->getName(), fileName);
    std::cout << "🗑️ Removed track: " << fileName << std::endl;
}

void PlaylistDetailView::play() {
    if (!controller || !playlist) return;

    controller->playPlaylist(playlist->getName());
}
