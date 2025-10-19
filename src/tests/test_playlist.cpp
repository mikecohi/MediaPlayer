#include "../model/PlaylistManager.h"
#include <iostream>

int main() {
    PlaylistManager pm;
    pm.loadMockData();

    const std::vector<Playlist>& lists = pm.getAllPlaylists();
    for (size_t i = 0; i < lists.size(); ++i) {
        lists[i].printToTerminal();
        std::cout << "--------------------" << std::endl;
    }
    return 0;
}
