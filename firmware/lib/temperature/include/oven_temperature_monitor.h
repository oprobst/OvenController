#pragma once

#include <cstdint>

#include "i_logger.h"
#include "i_temperature_sensor.h"
#include "i_thermocouple_bus.h"

/// ITemperatureSensor for the MAX31855 with plausibility check, debouncing and stale detection.
///
/// - Readings outside config::kPlausibleMinCelsius ... kPlausibleMaxCelsius count as faults (104).
/// - config::kFaultDebounceSamples faulty readings in a row set the error of the last one.
/// - config::kRecoverSamples valid readings in a row make the temperature valid again.
/// - No accepted value for config::kStaleTimeoutMs sets SensorNoResponse (105).
/// - Every change of the error is logged once (WARN "sensor fault" / INFO "sensor ok").
class OvenTemperatureMonitor final : public ITemperatureSensor {
public:
    /// @param bus raw converter access; must outlive the monitor
    /// @param logger diagnostics; must outlive the monitor
    OvenTemperatureMonitor(IThermocoupleBus& bus, ILogger& logger);

    void sample(Milliseconds now) override;
    OvenTemperature current() const override { return current_; }

private:
    void acceptValid(Celsius hot, Celsius cold, Milliseconds now);
    void setError(ErrorCode code);

    IThermocoupleBus& bus_;
    ILogger& logger_;
    OvenTemperature current_{false, Celsius{0.0f}, ErrorCode::None};
    uint8_t faultStreak_ = 0;
    uint8_t goodStreak_ = 0;
    bool started_ = false;
    Milliseconds lastAccepted_{0};
};
