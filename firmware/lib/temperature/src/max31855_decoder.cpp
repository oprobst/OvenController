#include "max31855_decoder.h"

namespace {

constexpr uint32_t kFaultBit = 1u << 16;
constexpr uint32_t kOpenCircuitBit = 1u << 0;
constexpr uint32_t kShortGndBit = 1u << 1;
constexpr uint32_t kShortVccBit = 1u << 2;
constexpr float kHotJunctionResolution = 0.25f;
constexpr float kColdJunctionResolution = 0.0625f;

}  // namespace

TemperatureReading Max31855Decoder::decode(uint32_t raw) {
    TemperatureReading reading{ErrorCode::None, Celsius{0.0f}, Celsius{0.0f}};

    if (raw == 0x00000000u || raw == 0xFFFFFFFFu) {
        reading.status = ErrorCode::SensorNoResponse;
        return reading;
    }

    // Bits 15..4: signed 12-bit cold junction temperature.
    const int16_t cold = static_cast<int16_t>(static_cast<uint16_t>(raw & 0xFFF0u)) >> 4;
    reading.coldJunction = Celsius{cold * kColdJunctionResolution};

    if ((raw & kFaultBit) != 0) {
        if ((raw & kOpenCircuitBit) != 0) {
            reading.status = ErrorCode::ThermocoupleOpen;
        } else if ((raw & kShortGndBit) != 0) {
            reading.status = ErrorCode::ThermocoupleShortGnd;
        } else if ((raw & kShortVccBit) != 0) {
            reading.status = ErrorCode::ThermocoupleShortVcc;
        } else {
            reading.status = ErrorCode::SensorNoResponse;
        }
        return reading;
    }

    // Bits 31..18: signed 14-bit hot junction temperature.
    const int32_t hot = static_cast<int32_t>(raw) >> 18;
    reading.hotJunction = Celsius{hot * kHotJunctionResolution};
    return reading;
}
