#pragma once

#include "error_code.h"
#include "units.h"

/// Debounced and plausibility-checked oven temperature as shown to the user.
struct OvenTemperature {
    bool valid;           ///< temperature may be displayed
    Celsius temperature;  ///< last accepted temperature
    ErrorCode error;      ///< active sensor error, or None
};
