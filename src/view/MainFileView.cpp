#include "view/MainFileView.h"
#include "model/MediaFile.h" // <-- Cần để vẽ
#include "model/Metadata.h"  // <-- Cần để vẽ
#include <cmath>
#include <algorithm>
#include <tuple> 

// ===================================
// SỬA LỖI 1: Sửa Constructor để khớp .h
// ===================================
MainFileView::MainFileView(NcursesUI* ui, WINDOW* win, MediaManager* manager)
    : ui(ui), win(win), mediaManager(manager), 
      filePage(1), fileSelected(0)
{
    // ===================================
    // SỬA LỖI 2: Xóa logic mockFiles cũ
    // ===================================
    // Bỏ mock data
    // for (int i = 1; i <= 100; ++i) {
    //    mockFiles.push_back("Mock File " + std::to_string(i) + ".mp3");
    // }

    // Tính toán phân trang
    int width;
    getmaxyx(win, std::ignore, width); 
    int height = getmaxy(win);
    
    itemsPerPage = height - 6; 
    if (itemsPerPage < 1) itemsPerPage = 1;
    
    // Lấy totalPages từ manager (dữ liệu thật)
    totalPages = mediaManager->getTotalPages(itemsPerPage);
    if (totalPages == 0) totalPages = 1; // Luôn là 1
}

void MainFileView::draw(FocusArea focus) {
    wclear(win); 
    box(win, 0, 0); 

    int width;
    getmaxyx(win, std::ignore, width);
    int listWidth = width / 2;

    // Lấy totalPages (có thể đã thay đổi)
    totalPages = mediaManager->getTotalPages(itemsPerPage);
    if (totalPages == 0) totalPages = 1;
    if (filePage > totalPages) filePage = totalPages; // Clamp

    // Vẽ panel danh sách (DỮ LIỆU THẬT)
    mvwprintw(win, 2, 3, "File List (Page %d/%d)", filePage, totalPages);
    
    // Lấy trang hiện tại từ manager
    std::vector<MediaFile*> filesOnPage = mediaManager->getPage(filePage, itemsPerPage); // <-- Đổi tên biến

    for (size_t i = 0; i < filesOnPage.size(); ++i) { // <-- Dùng filesOnPage.size()
        int fileIdxGlobal = (filePage - 1) * itemsPerPage + i;

        if (focus == FocusArea::MAIN_LIST && fileIdxGlobal == fileSelected) {
            wattron(win, A_REVERSE | A_BOLD);
        }
        
        // ===================================
        // SỬA LỖI 3: Dùng filesOnPage[i] thay vì mockFiles
        // ===================================
        mvwprintw(win, 4 + i, 3, "%.*s", listWidth - 5, filesOnPage[i]->getFileName().c_str());
        wattroff(win, A_REVERSE | A_BOLD);
    }

    // Vẽ panel chi tiết (DỮ LIỆU THẬT)
    mvwprintw(win, 2, listWidth + 2, "Metadata");
    if (focus == FocusArea::MAIN_DETAIL) wattron(win, A_REVERSE | A_BOLD);
    mvwprintw(win, 8, listWidth + 2, "[Edit Metadata]");
    wattroff(win, A_REVERSE | A_BOLD);
    
    // Lấy file được chọn (Dùng dữ liệu thật)
    int totalFiles = mediaManager->getTotalFileCount(); // <-- Lấy tổng số file
    if (fileSelected >= 0 && fileSelected < totalFiles) { // <-- Dùng totalFiles
        // (Cách lấy này chưa hiệu quả, nhưng đúng cho GĐ 3)
        std::vector<MediaFile*> pageData = mediaManager->getPage( // <-- Lấy lại page data
            (fileSelected / itemsPerPage) + 1, itemsPerPage
        );
        
        int indexOnPage = fileSelected % itemsPerPage;
        if (indexOnPage < (int)pageData.size()) { // Kiểm tra index hợp lệ trên trang
             MediaFile* selectedFile = pageData[indexOnPage];
             if (selectedFile && selectedFile->getMetadata()) {
                 mvwprintw(win, 4, listWidth + 2, "Title: %s", selectedFile->getMetadata()->title.c_str());
             }
        }
        
    } else {
         mvwprintw(win, 4, listWidth + 2, "Title: N/A");
    }
    
    wrefresh(win); // Đẩy ra màn hình
}

void MainFileView::handleInput(InputEvent event, FocusArea focus) {
    int totalFiles = mediaManager->getTotalFileCount(); // <-- Dùng manager
    if (focus == FocusArea::MAIN_LIST && totalFiles > 0) {
        if (event.key == KEY_DOWN) fileSelected = std::min(fileSelected + 1, totalFiles - 1); // <-- Dùng totalFiles
        if (event.key == KEY_UP) fileSelected = std::max(0, fileSelected - 1);
        
        // (TODO: Thêm logic chuyển trang khi nhấn Lên/Xuống)
    }
}