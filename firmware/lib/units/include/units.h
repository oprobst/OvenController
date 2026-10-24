#pragma once

#include <cmath>
#include <cstdint>

/// Temperature in degrees Celsius.
struct Celsius {
    float value;

    /// @return value rounded half away from zero to whole degrees Celsius (23.5 -> 24, -0.5 -> -1).
    int32_t roundedWholeDegrees() const { return static_cast<int32_t>(std::lround(value)); }
};

/// Point in time in milliseconds since start. Wraps around after about 49 days.
struct Milliseconds {
    uint32_t value;

    /// @return milliseconds from earlier to this point in time; correct across one wrap-around.
    uint32_t elapsedSince(Milliseconds earlier) const { return value - earlier.value; }
};
