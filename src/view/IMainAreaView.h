#pragma once
#include "utils/NcursesUI.h"
#include "view/UIManager.h" // Needs FocusArea

class IMainAreaView {
public:
    virtual ~IMainAreaView() = default;
    virtual void draw(FocusArea focus) = 0;
    virtual void handleInput(InputEvent event, FocusArea focus) = 0;
    virtual void handleMouse(int localY, int localX) = 0;
};