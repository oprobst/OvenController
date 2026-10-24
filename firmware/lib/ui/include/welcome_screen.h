#pragma once

#include "firmware_identity.h"
#include "i_graphics.h"

/// Content of the welcome screen (below the status area).
class WelcomeScreen {
public:
    /// @param graphics drawing target; must outlive the screen
    explicit WelcomeScreen(IGraphics& graphics);

    /// Draws greeting, title, firmware version and PCB revision. Expects a cleared content area.
    void draw(const FirmwareIdentity& identity);

private:
    IGraphics& graphics_;
};
