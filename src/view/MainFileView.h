#pragma once
#include "view/IMainAreaView.h"
#include <vector>
#include <string>
#include "model/MediaManager.h"
#include "model/MediaFile.h"

class MainFileView : public IMainAreaView {
public:
    MainFileView(NcursesUI* ui, WINDOW* win, MediaManager* manager);
    void draw(FocusArea focus) override;
    void handleInput(InputEvent event, FocusArea focus) override;
    void handleMouse(int localY, int localX) override;
    MediaFile* getSelectedFile() const;

private:
    NcursesUI* ui;
    WINDOW* win;
    MediaManager* mediaManager;
    int filePage, totalPages, itemsPerPage, fileSelected;
};