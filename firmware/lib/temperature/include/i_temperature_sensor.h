#pragma once

#include "oven_temperature.h"
#include "units.h"

/// Evaluated oven temperature for display and, later, control.
class ITemperatureSensor {
public:
    virtual ~ITemperatureSensor() = default;

    /// Takes one measurement and updates the state. Call every config::kTemperatureSamplePeriodMs.
    /// @param now current time
    virtual void sample(Milliseconds now) = 0;

    /// @return current state; error != None if the sensor is faulty or the value is implausible.
    virtual OvenTemperature current() const = 0;
};
