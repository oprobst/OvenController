#pragma once

#include "wall_clock_time.h"

/// Local time obtained from the network (FR-004, FR-005).
class IWallClock {
public:
    virtual ~IWallClock() = default;

    /// Configures time zone and time servers. Call after INetworkStatus::begin(). Returns
    /// immediately.
    virtual void begin() = 0;

    /// Detects and logs the first synchronization. Call periodically from the main loop.
    virtual void poll() = 0;

    /// @return current local time; valid == false until the first synchronization.
    virtual WallClockTime now() const = 0;
};
