#pragma once

#include "units.h"

/// Monotonic time source.
class IClock {
public:
    virtual ~IClock() = default;

    /// @return time since start in ms; wraps after about 49 days (differences stay valid).
    virtual Milliseconds now() const = 0;
};
