#pragma once

#include "i_temperature_sensor.h"

/// Temperature sensor whose state is set by the test.
class FakeTemperatureSensor final : public ITemperatureSensor {
public:
    void sample(Milliseconds /*now*/) override { ++samples; }
    OvenTemperature current() const override { return state; }

    OvenTemperature state{true, Celsius{23.0f}, ErrorCode::None};
    int samples = 0;
};
