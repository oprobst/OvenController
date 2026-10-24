#pragma once

#include "i_clock.h"

/// Clock whose time is set by the test.
class FakeClock final : public IClock {
public:
    Milliseconds now() const override { return current; }

    Milliseconds current{0};
};
