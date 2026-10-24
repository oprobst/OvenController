#pragma once

#include "i_thermocouple_bus.h"

/// IThermocoupleBus for the MAX31855 on its own bit-banged lines (pins from board.h).
/// Independent of the display SPI bus, so a display transfer never delays a measurement.
class Esp32Max31855Bus final : public IThermocoupleBus {
public:
    /// Configures the pins (CS high, SCK low). Call once during setup.
    void begin();

    uint32_t readRaw() override;
};
