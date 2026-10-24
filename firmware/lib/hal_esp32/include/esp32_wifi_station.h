#pragma once

#include <atomic>
#include <cstdint>

#include "i_logger.h"
#include "i_network_status.h"

/// INetworkStatus using the ESP32 WiFi station with credentials from wifi_secrets.h.
/// Without wifi_secrets.h, or with an empty SSID, the state is NotConfigured and WiFi stays off.
/// WiFi events (event task) only update atomics; logging happens in poll() (main loop).
class Esp32WifiStation final : public INetworkStatus {
public:
    /// @param logger diagnostics; must outlive the station
    explicit Esp32WifiStation(ILogger& logger);

    void begin() override;
    void poll() override;
    WifiState state() const override { return state_.load(); }

private:
    ILogger& logger_;
    std::atomic<WifiState> state_{WifiState::Disconnected};
    std::atomic<uint8_t> lastDisconnectReason_{0};
    WifiState loggedState_ = WifiState::Disconnected;
};
