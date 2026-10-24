#pragma once

#include <cstdint>
#include <deque>

#include "i_thermocouple_bus.h"

/// Returns queued raw words; repeats the last word when the queue is empty.
class FakeThermocoupleBus final : public IThermocoupleBus {
public:
    uint32_t readRaw() override {
        if (!queue.empty()) {
            last = queue.front();
            queue.pop_front();
        }
        ++reads;
        return last;
    }

    /// Raw word for a valid reading (hot junction in whole degC, cold junction 25 degC).
    static uint32_t valid(int32_t celsius) {
        return ((static_cast<uint32_t>(celsius * 4) & 0x3FFFu) << 18) | (400u << 4);
    }

    /// Raw word with fault bit and detail bits (1 = open, 2 = short GND, 4 = short VCC).
    static uint32_t fault(uint32_t detailBits) { return valid(25) | (1u << 16) | detailBits; }

    std::deque<uint32_t> queue;
    uint32_t last = 0;
    int reads = 0;
};
