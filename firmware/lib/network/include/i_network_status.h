#pragma once

#include "wifi_state.h"

/// WiFi connection (FR-003, FR-008).
class INetworkStatus {
public:
    virtual ~INetworkStatus() = default;

    /// Starts connecting in the background. Returns immediately.
    virtual void begin() = 0;

    /// Processes pending connection events and logs state changes. Call periodically from the
    /// main loop (config::kNetworkPollPeriodMs). Never blocks.
    virtual void poll() = 0;

    /// @return current state.
    virtual WifiState state() const = 0;
};
