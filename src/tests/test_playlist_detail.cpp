#include "../view/PlaylistDetailView.h"
#include "../controller/PlaylistController.h"
#include "../model/PlaylistManager.h"
#include "../controller/MediaController.h"

int main() {
    PlaylistManager pm;
    MediaController mc;
    PlaylistController controller(&pm, &mc);

    Playlist* p = pm.createPlaylist("TestPlaylist");
    PlaylistDetailView view(p, &controller);
    view.show();
}
