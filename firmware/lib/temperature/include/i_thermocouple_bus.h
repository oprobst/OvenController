#pragma once

#include <cstdint>

/// Raw access to the thermocouple converter (HAL).
class IThermocoupleBus {
public:
    virtual ~IThermocoupleBus() = default;

    /// Reads one 32-bit raw word of the MAX31855 (MSB first). Takes less than 50 us.
    /// @return raw word; 0x00000000 or 0xFFFFFFFF if the converter does not answer
    virtual uint32_t readRaw() = 0;
};
