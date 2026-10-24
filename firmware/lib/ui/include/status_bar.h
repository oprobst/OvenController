#pragma once

#include "i_graphics.h"
#include "status_formatter.h"

/// Draws the two-row status area and the sensor fault line. Redraws a region only when its
/// text or color changed since it was last drawn.
class StatusBar {
public:
    /// @param graphics drawing target; must outlive the status bar
    explicit StatusBar(IGraphics& graphics);

    /// Draws all regions whose content changed.
    void draw(const StatusTexts& texts);

    /// Forces a full redraw on the next draw(), e.g. after the content area was cleared.
    void invalidate();

private:
    struct Region {
        StatusText shown;
        bool drawn;
    };

    void drawRegion(Region& region, const StatusText& text, int16_t x, int16_t y, uint16_t w,
                    uint16_t h, FontSize size, Align align);

    IGraphics& graphics_;
    bool separatorDrawn_ = false;
    Region wifi_{};
    Region time_{};
    Region date_{};
    Region temperature_{};
    Region faultLine_{};
};
