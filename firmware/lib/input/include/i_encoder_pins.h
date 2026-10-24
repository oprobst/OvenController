#pragma once

#include <cstdint>

/// Pin access of the rotary encoder (HAL).
class IEncoderPins {
public:
    virtual ~IEncoderPins() = default;

    /// @return detents accumulated since the last call (+ = clockwise); read atomically.
    virtual int32_t takeDetents() = 0;

    /// @return true while the push button is pressed (raw level, not debounced).
    virtual bool buttonLevelPressed() const = 0;
};
