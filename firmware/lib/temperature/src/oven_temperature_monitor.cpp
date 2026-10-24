#include "oven_temperature_monitor.h"

#include <cstdio>

#include "app_config.h"
#include "max31855_decoder.h"

namespace {

constexpr char kModule[] = "temp";

}  // namespace

OvenTemperatureMonitor::OvenTemperatureMonitor(IThermocoupleBus& bus, ILogger& logger)
    : bus_(bus), logger_(logger) {}

void OvenTemperatureMonitor::sample(Milliseconds now) {
    if (!started_) {
        started_ = true;
        lastAccepted_ = now;
    }

    const TemperatureReading reading = Max31855Decoder::decode(bus_.readRaw());
    ErrorCode code = reading.status;
    if (code == ErrorCode::None && (reading.hotJunction.value < config::kPlausibleMinCelsius ||
                                    reading.hotJunction.value > config::kPlausibleMaxCelsius)) {
        code = ErrorCode::TemperatureImplausible;
    }

    if (code == ErrorCode::None) {
        faultStreak_ = 0;
        if (goodStreak_ < UINT8_MAX) {
            ++goodStreak_;
        }
        if (current_.valid || goodStreak_ >= config::kRecoverSamples) {
            acceptValid(reading.hotJunction, reading.coldJunction, now);
        }
    } else {
        goodStreak_ = 0;
        if (faultStreak_ < UINT8_MAX) {
            ++faultStreak_;
        }
        if (faultStreak_ >= config::kFaultDebounceSamples) {
            setError(code);
        }
    }

    if (current_.error == ErrorCode::None &&
        now.elapsedSince(lastAccepted_) >= config::kStaleTimeoutMs) {
        setError(ErrorCode::SensorNoResponse);
    }
}

void OvenTemperatureMonitor::acceptValid(Celsius hot, Celsius cold, Milliseconds now) {
    const bool recovered = current_.error != ErrorCode::None;
    current_.valid = true;
    current_.temperature = hot;
    current_.error = ErrorCode::None;
    lastAccepted_ = now;
    if (recovered) {
        char fields[48];
        std::snprintf(fields, sizeof(fields), "temp_c=%.2f cj_c=%.2f",
                      static_cast<double>(hot.value), static_cast<double>(cold.value));
        logger_.log(LogLevel::Info, kModule, "sensor ok", fields);
    }
}

void OvenTemperatureMonitor::setError(ErrorCode code) {
    current_.valid = false;
    if (current_.error == code) {
        return;
    }
    current_.error = code;
    char codeText[8];
    formatErrorCode(code, codeText, sizeof(codeText));
    char fields[16];
    std::snprintf(fields, sizeof(fields), "code=%s", codeText);
    logger_.log(LogLevel::Warn, kModule, "sensor fault", fields);
}
