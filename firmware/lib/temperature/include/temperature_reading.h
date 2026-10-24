#pragma once

#include "error_code.h"
#include "units.h"

/// Result of a single MAX31855 conversion.
struct TemperatureReading {
    ErrorCode status;      ///< None, or 101/102/103/105
    Celsius hotJunction;   ///< oven temperature (0.25 degC resolution); valid if status == None
    Celsius coldJunction;  ///< converter temperature (0.0625 degC resolution); for logging only
};
