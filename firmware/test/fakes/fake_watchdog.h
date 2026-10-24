#pragma once

#include "i_watchdog.h"

/// Counts watchdog calls.
class FakeWatchdog final : public IWatchdog {
public:
    void start() override { ++starts; }
    void feed() override { ++feeds; }

    int starts = 0;
    int feeds = 0;
};
