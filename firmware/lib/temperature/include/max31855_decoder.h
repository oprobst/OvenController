#pragma once

#include <cstdint>

#include "temperature_reading.h"

/// Decodes the 32-bit raw word of a MAX31855K thermocouple converter.
class Max31855Decoder {
public:
    /// @param raw raw word as read from the converter, MSB first
    /// @return reading; status is SensorNoResponse for 0x00000000/0xFFFFFFFF or a fault bit
    ///         without detail, ThermocoupleOpen/ShortGnd/ShortVcc for the fault detail bits
    static TemperatureReading decode(uint32_t raw);
};
