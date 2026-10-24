#pragma once

#include <cstdint>

/// WiFi state as shown to the user (FR-003).
enum class WifiState : uint8_t {
    NotConfigured,  ///< no credentials compiled in
    Disconnected,   ///< credentials present, no connection (also while connecting)
    Connected,      ///< IP address obtained
};
