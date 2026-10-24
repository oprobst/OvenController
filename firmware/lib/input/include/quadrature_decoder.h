#pragma once

#include <cstdint>

/// Decodes quadrature transitions of a rotary encoder. Pure logic, usable from an ISR if the
/// table is copied into RAM by the caller.
class QuadratureDecoder {
public:
    /// Steps indexed by (previousAB << 2) | currentAB, with AB = (A << 1) | B.
    /// +1 = clockwise, -1 = counter-clockwise, 0 = no change or invalid double jump.
    static constexpr int8_t kTransitionTable[16] = {0,  +1, -1, 0,  -1, 0,  0,  +1,
                                                    +1, 0,  0,  -1, 0,  -1, +1, 0};

    /// @param previousAB previous pin state (0..3)
    /// @param currentAB current pin state (0..3)
    /// @return step: -1, 0 or +1
    static constexpr int8_t step(uint8_t previousAB, uint8_t currentAB) {
        return kTransitionTable[((previousAB & 0x3u) << 2) | (currentAB & 0x3u)];
    }
};
