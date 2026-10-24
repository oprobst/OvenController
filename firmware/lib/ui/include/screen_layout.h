#pragma once

#include <cstdint>

/// Screen layout for the 240 x 320 portrait display.
/// Source: specs/001-startup-status-menu/contracts/ui-screens.md
namespace layout {

/// Rectangle in pixels.
struct Box {
    int16_t x;
    int16_t y;
    uint16_t w;
    uint16_t h;
};

constexpr uint16_t kScreenWidth = 240;
constexpr uint16_t kScreenHeight = 320;

// Status area: two rows of 24 px and a separator line.
constexpr Box kStatusWifi{0, 0, 170, 24};
constexpr Box kStatusTime{170, 0, 70, 24};
constexpr Box kStatusDate{0, 24, 120, 24};
constexpr Box kStatusTemperature{120, 24, 120, 24};
constexpr Box kSeparator{0, 48, kScreenWidth, 1};

// Content area below the status area; cleared on every screen change.
constexpr int16_t kContentTop = 49;
constexpr Box kContent{0, kContentTop, kScreenWidth, kScreenHeight - kContentTop};

// Sensor fault line at the bottom of the content area.
constexpr Box kFaultLine{0, 292, kScreenWidth, 28};

// Welcome screen.
constexpr Box kWelcomeGreeting{0, 92, kScreenWidth, 36};
constexpr Box kWelcomeTitle{0, 132, kScreenWidth, 28};
constexpr Box kWelcomeFirmware{0, 210, kScreenWidth, 20};
constexpr Box kWelcomePcb{0, 230, kScreenWidth, 20};

// Main menu.
constexpr Box kMenuHeader{0, 56, kScreenWidth, 24};
constexpr int16_t kMenuFirstEntryY = 84;
constexpr uint16_t kMenuEntryHeight = 40;
constexpr uint8_t kMenuVisibleEntries = 5;

}  // namespace layout
