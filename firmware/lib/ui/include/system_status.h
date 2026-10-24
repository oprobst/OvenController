#pragma once

#include "oven_temperature.h"
#include "wall_clock_time.h"
#include "wifi_state.h"

/// Snapshot of everything shown in the status area.
struct SystemStatus {
    WifiState wifi;
    WallClockTime clock;
    OvenTemperature oven;
};
