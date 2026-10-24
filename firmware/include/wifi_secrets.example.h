#pragma once

// Copy to wifi_secrets.h and fill in; never commit wifi_secrets.h (it is listed in .gitignore).
// Without wifi_secrets.h, or with an empty SSID, the firmware reports "WiFi not configured".

namespace secrets {

constexpr char kWifiSsid[] = "";
constexpr char kWifiPassword[] = "";

}  // namespace secrets
