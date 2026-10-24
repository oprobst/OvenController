#pragma once

#include <functional>

#include "i_network_status.h"
#include "i_wall_clock.h"

/// Network status set by the test; onBegin lets a test observe when begin() is called.
class FakeNetworkStatus final : public INetworkStatus {
public:
    void begin() override {
        ++begins;
        if (onBegin) {
            onBegin();
        }
    }
    void poll() override { ++polls; }
    WifiState state() const override { return current; }

    WifiState current = WifiState::Disconnected;
    std::function<void()> onBegin;
    int begins = 0;
    int polls = 0;
};

/// Wall clock set by the test.
class FakeWallClock final : public IWallClock {
public:
    void begin() override { ++begins; }
    void poll() override { ++polls; }
    WallClockTime now() const override { return current; }

    WallClockTime current{false, 1970, 1, 1, 0, 0};
    int begins = 0;
    int polls = 0;
};
