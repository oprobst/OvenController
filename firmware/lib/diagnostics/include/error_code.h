#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>

/// Central error codes (constitution principle IX). Values are stable: never reinterpret or
/// reuse a code. Documented in docs/error-codes.md.
enum class ErrorCode : uint16_t {
    None = 0,
    ThermocoupleOpen = 101,        ///< thermocouple not connected or wire broken
    ThermocoupleShortGnd = 102,    ///< thermocouple shorted to GND
    ThermocoupleShortVcc = 103,    ///< thermocouple shorted to VCC
    TemperatureImplausible = 104,  ///< reading outside the plausible range
    SensorNoResponse = 105,        ///< converter does not answer or reading is stale
};

/// Formats an error code for display, e.g. "E101".
/// @param out destination buffer, at least 5 bytes
/// @param len size of out in bytes; output is truncated and null-terminated if too small
inline void formatErrorCode(ErrorCode code, char* out, size_t len) {
    std::snprintf(out, len, "E%03u", static_cast<unsigned>(code));
}
