#pragma once

#include <deque>

#include "i_encoder_pins.h"
#include "i_rotary_input.h"

/// Encoder pins whose detents and button level are set by the test.
class FakeEncoderPins final : public IEncoderPins {
public:
    int32_t takeDetents() override {
        const int32_t result = detents;
        detents = 0;
        return result;
    }
    bool buttonLevelPressed() const override { return pressed; }

    int32_t detents = 0;
    bool pressed = false;
};

/// Rotary input delivering queued events.
class FakeRotaryInput final : public IRotaryInput {
public:
    void poll(Milliseconds /*now*/) override { ++polls; }
    InputEvent nextEvent() override {
        if (events.empty()) {
            return InputEvent::None;
        }
        const InputEvent event = events.front();
        events.pop_front();
        return event;
    }

    std::deque<InputEvent> events;
    int polls = 0;
};
