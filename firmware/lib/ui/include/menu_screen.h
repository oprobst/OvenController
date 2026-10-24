#pragma once

#include <cstddef>

#include "i_graphics.h"
#include "main_menu.h"

/// Content of the main menu screen (below the status area).
class MenuScreen {
public:
    /// @param graphics drawing target; must outlive the screen
    explicit MenuScreen(IGraphics& graphics);

    /// Draws header and up to layout::kMenuVisibleEntries entries. Expects a cleared content area.
    void draw(const MainMenu& menu);

    /// Redraws only the previously and the newly marked entry if the cursor moved.
    void update(const MainMenu& menu);

private:
    void drawEntry(const MainMenu& menu, size_t index);

    IGraphics& graphics_;
    size_t shownCursor_ = 0;
};
