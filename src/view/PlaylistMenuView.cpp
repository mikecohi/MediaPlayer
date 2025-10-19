#include "PlaylistMenuView.h"
#include "../controller/PlaylistController.h"

PlaylistMenuView::PlaylistMenuView(PlaylistManager* pm)
    : playlistManager(pm), controller(NULL) {}

void PlaylistMenuView::setController(PlaylistController* ctrl) {
    controller = ctrl;
}

void PlaylistMenuView::displayMenu() {
    std::cout << "\n===== Playlist Menu =====" << std::endl;
    std::cout << "1. Add track to playlist" << std::endl;
    std::cout << "2. Remove track from playlist" << std::endl;
    std::cout << "3. Play playlist" << std::endl;
    std::cout << "4. Exit\n";
    std::cout << "===========================" << std::endl;

    int choice;
    std::cout << "Enter choice: ";
    std::cin >> choice;
    std::cin.ignore();

    if (controller) controller->handleUserChoice(choice);
}
