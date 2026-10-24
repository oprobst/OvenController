#pragma once

#include <atomic>
#include <cstdint>

#include "i_encoder_pins.h"

/// IEncoderPins for a mechanical rotary encoder with push button (pins from board.h).
/// A and B are decoded in a GPIO interrupt; the ISR only uses data in RAM (copied transition
/// table, atomic counter) so it stays valid while the flash cache is disabled.
class Esp32EncoderPins final : public IEncoderPins {
public:
    Esp32EncoderPins();

    /// Configures the pins with internal pull-ups and attaches the interrupts. Call once.
    void begin();

    int32_t takeDetents() override;
    bool buttonLevelPressed() const override;

private:
    static void onEdge(void* self);

    int8_t transitionTable_[16];
    volatile uint8_t previousAB_ = 0;
    std::atomic<int32_t> steps_{0};
};
