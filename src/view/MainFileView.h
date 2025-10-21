#pragma once
#include "view/IMainAreaView.h"
#include <vector>
#include <string>

// Include Model thật, bỏ mock data
#include "model/MediaManager.h"

class MainFileView : public IMainAreaView {
public:
    // Sửa Constructor: Nhận MediaManager
    MainFileView(NcursesUI* ui, WINDOW* win, MediaManager* manager);
    void draw(FocusArea focus) override;
    void handleInput(InputEvent event, FocusArea focus) override;

private:
    NcursesUI* ui;
    WINDOW* win;
    MediaManager* mediaManager; // <-- Con trỏ (không sở hữu) đến Model
    
    // Bỏ mock data
    int filePage, totalPages, itemsPerPage, fileSelected;
};