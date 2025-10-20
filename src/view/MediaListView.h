#pragma once
#include "view/IView.h"
#include "utils/NcursesUI.h"
#include <vector>
#include <string>

/**
 * @class MediaListView
 * @brief Implements the IView interface for the media list screen.
 * This version uses MOCK DATA.
 */
class MediaListView : public IView {
public:
    explicit MediaListView(NcursesUI* ui);
    ~MediaListView();

    void draw() override;
    ViewId handleInput(InputEvent event) override;

private:
    void updatePagination();

    NcursesUI* ui; // Non-owning pointer
    WINDOW* win;   // The ncurses window for this view

    // --- MOCK DATA ---
    std::vector<std::string> mockMediaList;

    // --- State for navigation ---
    int selectedIndex;  // Index in the *full* mock list
    int currentPage;
    int totalPages;
    int itemsPerPage;
};