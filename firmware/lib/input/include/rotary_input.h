#pragma once

#include <cstddef>

#include "button_debouncer.h"
#include "i_encoder_pins.h"
#include "i_rotary_input.h"

/// IRotaryInput turning encoder detents and debounced button presses into events.
/// Holds up to kQueueSize events; further events are dropped until the queue is read.
class RotaryInput final : public IRotaryInput {
public:
    static constexpr size_t kQueueSize = 16;

    /// @param pins encoder pin access; must outlive the input
    explicit RotaryInput(IEncoderPins& pins);

    void poll(Milliseconds now) override;
    InputEvent nextEvent() override;

private:
    void push(InputEvent event);

    IEncoderPins& pins_;
    ButtonDebouncer debouncer_;
    InputEvent queue_[kQueueSize] = {};
    size_t head_ = 0;
    size_t size_ = 0;
};
