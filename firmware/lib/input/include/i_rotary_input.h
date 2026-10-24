#pragma once

#include "input_event.h"
#include "units.h"

/// Operator events of the rotary encoder with push button.
class IRotaryInput {
public:
    virtual ~IRotaryInput() = default;

    /// Samples the button and collects rotation steps. Call every config::kInputPollPeriodMs.
    /// @param now current time
    virtual void poll(Milliseconds now) = 0;

    /// @return next event, or None if there is none. Events are delivered in order.
    virtual InputEvent nextEvent() = 0;
};
