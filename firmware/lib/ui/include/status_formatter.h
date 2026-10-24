#pragma once

#include <cstddef>

#include "i_graphics.h"
#include "system_status.h"

/// One formatted text with its color.
struct StatusText {
    static constexpr size_t kCapacity = 32;  ///< bytes including terminator (UTF-8)

    char text[kCapacity];
    Color color;
};

/// All texts of the status area plus the sensor fault line.
struct StatusTexts {
    StatusText wifi;
    StatusText time;
    StatusText date;
    StatusText temperature;
    StatusText faultLine;  ///< empty if there is no sensor fault
};

/// Turns a SystemStatus into display texts (rules: contracts/ui-screens.md).
class StatusFormatter {
public:
    /// @return texts; time and date are never shown for a year before config::kMinValidYear
    static StatusTexts format(const SystemStatus& status);
};
