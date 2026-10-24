#pragma once

#include <cstdint>

#include "units.h"

/// Debounces a push button. A level must be stable for the debounce time before it counts.
class ButtonDebouncer {
public:
    /// @param debounceMs required stable time in ms
    explicit ButtonDebouncer(uint32_t debounceMs);

    /// Feeds the current raw level.
    /// @param levelPressed true if the button is pressed right now
    /// @param now current time
    /// @return true exactly once per press (on the debounced transition to pressed). The level
    ///         at the first call is taken as stable without an event.
    bool update(bool levelPressed, Milliseconds now);

private:
    uint32_t debounceMs_;
    bool initialized_ = false;
    bool rawLevel_ = false;
    bool stableLevel_ = false;
    Milliseconds lastChange_{0};
};
