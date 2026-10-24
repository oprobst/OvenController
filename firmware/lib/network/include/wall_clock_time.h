#pragma once

#include <cstdint>

/// Local date and time (CET/CEST).
struct WallClockTime {
    bool valid;      ///< true after the first time synchronization with year >= kMinValidYear
    uint16_t year;   ///< e.g. 2026
    uint8_t month;   ///< 1..12
    uint8_t day;     ///< 1..31
    uint8_t hour;    ///< 0..23
    uint8_t minute;  ///< 0..59
};
