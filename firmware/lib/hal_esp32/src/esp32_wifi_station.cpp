#include "esp32_wifi_station.h"

#include <WiFi.h>

#include <cstdio>

#if __has_include("wifi_secrets.h")
#include "wifi_secrets.h"
namespace {
constexpr const char* kSsid = secrets::kWifiSsid;
constexpr const char* kPassword = secrets::kWifiPassword;
}  // namespace
#else
namespace {
constexpr const char* kSsid = "";
constexpr const char* kPassword = "";
}  // namespace
#endif

namespace {

constexpr char kModule[] = "wifi";

const char* disconnectReasonName(uint8_t reason) {
    switch (reason) {
        case WIFI_REASON_BEACON_TIMEOUT:
            return "beacon_timeout";
        case WIFI_REASON_NO_AP_FOUND:
            return "no_ap_found";
        case WIFI_REASON_AUTH_FAIL:
            return "auth_fail";
        case WIFI_REASON_ASSOC_FAIL:
            return "assoc_fail";
        case WIFI_REASON_HANDSHAKE_TIMEOUT:
            return "handshake_timeout";
        case WIFI_REASON_AUTH_EXPIRE:
            return "auth_expire";
        default:
            return nullptr;
    }
}

}  // namespace

Esp32WifiStation::Esp32WifiStation(ILogger& logger) : logger_(logger) {}

void Esp32WifiStation::begin() {
    if (kSsid[0] == '\0') {
        state_ = WifiState::NotConfigured;
        loggedState_ = WifiState::NotConfigured;
        logger_.log(LogLevel::Warn, kModule, "not configured");
        return;
    }

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
        if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP) {
            state_ = WifiState::Connected;
        } else if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
            lastDisconnectReason_ = info.wifi_sta_disconnected.reason;
            state_ = WifiState::Disconnected;
        }
    });
    WiFi.persistent(false);  // no flash writes: credentials come from the firmware
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(kSsid, kPassword);
}

void Esp32WifiStation::poll() {
    const WifiState current = state_.load();
    if (current == loggedState_) {
        return;
    }
    loggedState_ = current;
    char fields[48];
    if (current == WifiState::Connected) {
        std::snprintf(fields, sizeof(fields), "rssi=%d", static_cast<int>(WiFi.RSSI()));
        logger_.log(LogLevel::Info, kModule, "connected", fields);
    } else if (current == WifiState::Disconnected) {
        const uint8_t reason = lastDisconnectReason_.load();
        const char* name = disconnectReasonName(reason);
        if (name != nullptr) {
            std::snprintf(fields, sizeof(fields), "reason=%s", name);
        } else {
            std::snprintf(fields, sizeof(fields), "reason=%u", static_cast<unsigned>(reason));
        }
        logger_.log(LogLevel::Warn, kModule, "disconnected", fields);
    }
}
