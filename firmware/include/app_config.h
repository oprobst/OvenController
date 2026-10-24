#pragma once

// Central application constants (constitution principle VII). No magic numbers elsewhere.

#include <cstddef>
#include <cstdint>

namespace config {

// User interface
constexpr uint32_t kWelcomeDurationMs = 3000;
constexpr uint32_t kDisplayRefreshPeriodMs = 100;
constexpr uint32_t kInputPollPeriodMs = 5;
constexpr uint32_t kButtonDebounceMs = 20;
constexpr size_t kMaxMenuEntries = 8;

// Safety and supervision
constexpr uint32_t kWatchdogTimeoutMs = 5000;

// Oven temperature (MAX31855)
constexpr uint32_t kTemperatureSamplePeriodMs = 250;
constexpr float kPlausibleMinCelsius = -20.0f;
constexpr float kPlausibleMaxCelsius = 350.0f;
constexpr uint8_t kFaultDebounceSamples = 2;
constexpr uint8_t kRecoverSamples = 2;
constexpr uint32_t kStaleTimeoutMs = 1000;

// Network and time
constexpr uint32_t kNetworkPollPeriodMs = 1000;
constexpr uint16_t kMinValidYear = 2026;
constexpr char kTimeZonePosix[] = "CET-1CEST,M3.5.0,M10.5.0/3";
constexpr char kNtpServer1[] = "de.pool.ntp.org";
constexpr char kNtpServer2[] = "pool.ntp.org";

// Diagnostics
constexpr size_t kLogLineMaxChars = 255;
constexpr uint32_t kSerialBaud = 115200;
constexpr size_t kSerialTxBufferBytes = 2048;

}  // namespace config
