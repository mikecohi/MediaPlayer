#pragma once
#include "utils/NcursesUI.h"
#include "view/UIManager.h" // Cần FocusArea

/**
 * @class IMainAreaView
 * @brief Interface cho các panel nội dung chính (File, Playlist).
 */
class IMainAreaView {
public:
    virtual ~IMainAreaView() = default;

    /**
     * @brief Vẽ nội dung vào cửa sổ chính (mainWin).
     * @param focus Khu vực con nào đang được focus (LIST hay DETAIL).
     */
    virtual void draw(FocusArea focus) = 0;

    /**
     * @brief Xử lý input cho panel chính.
     */
    virtual void handleInput(InputEvent event, FocusArea focus) = 0;
};