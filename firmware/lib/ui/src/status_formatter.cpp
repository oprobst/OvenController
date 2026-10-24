#include "status_formatter.h"

#include <cstdio>

#include "app_config.h"

namespace {

constexpr char kDegreeCelsius[] =
    "\xC2\xB0"
    "C";

void set(StatusText& target, const char* text, Color color) {
    std::snprintf(target.text, sizeof(target.text), "%s", text);
    target.color = color;
}

void formatWifi(WifiState state, StatusText& out) {
    switch (state) {
        case WifiState::Connected:
            set(out, "WLAN verbunden", Color::Foreground);
            return;
        case WifiState::Disconnected:
            set(out, "WLAN nicht verbunden", Color::Warning);
            return;
        case WifiState::NotConfigured:
            set(out, "WLAN nicht eingerichtet", Color::Muted);
            return;
    }
    set(out, "", Color::Muted);
}

void formatClock(WifiState wifi, const WallClockTime& clock, StatusText& time, StatusText& date) {
    if (clock.valid && clock.year >= config::kMinValidYear) {
        std::snprintf(time.text, sizeof(time.text), "%02u:%02u", clock.hour, clock.minute);
        std::snprintf(date.text, sizeof(date.text), "%02u.%02u.%04u", clock.day, clock.month,
                      clock.year);
        time.color = Color::Foreground;
        date.color = Color::Foreground;
    } else if (wifi == WifiState::Connected) {
        set(time, "--:--", Color::Muted);
        set(date, "--.--.----", Color::Muted);
    } else {
        set(time, "", Color::Muted);
        set(date, "", Color::Muted);
    }
}

void formatTemperature(const OvenTemperature& oven, StatusText& temperature, StatusText& fault) {
    if (oven.error != ErrorCode::None) {
        char code[8];
        formatErrorCode(oven.error, code, sizeof(code));
        set(temperature, code, Color::Error);
        std::snprintf(fault.text, sizeof(fault.text), "F\xC3\xBChlerfehler %s", code);
        fault.color = Color::Error;
        return;
    }
    set(fault, "", Color::Error);
    if (oven.valid) {
        std::snprintf(temperature.text, sizeof(temperature.text), "%ld %s",
                      static_cast<long>(oven.temperature.roundedWholeDegrees()), kDegreeCelsius);
        temperature.color = Color::Foreground;
    } else {
        std::snprintf(temperature.text, sizeof(temperature.text), "--- %s", kDegreeCelsius);
        temperature.color = Color::Muted;
    }
}

}  // namespace

StatusTexts StatusFormatter::format(const SystemStatus& status) {
    StatusTexts texts{};
    formatWifi(status.wifi, texts.wifi);
    formatClock(status.wifi, status.clock, texts.time, texts.date);
    formatTemperature(status.oven, texts.temperature, texts.faultLine);
    return texts;
}
