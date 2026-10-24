// HIL test: the MAX31855 delivers plausible readings at room temperature.
// Precondition: thermocouple connected, oven cold (10..40 degC).

#include <Arduino.h>
#include <unity.h>

#include "esp32_max31855_bus.h"
#include "max31855_decoder.h"

namespace {

Esp32Max31855Bus bus;

void test_ten_readings_at_room_temperature() {
    for (int i = 0; i < 10; ++i) {
        const TemperatureReading reading = Max31855Decoder::decode(bus.readRaw());
        TEST_ASSERT_EQUAL_UINT16(0, static_cast<uint16_t>(reading.status));
        TEST_ASSERT_FLOAT_WITHIN(15.0f, 25.0f, reading.hotJunction.value);
        TEST_ASSERT_FLOAT_WITHIN(15.0f, 25.0f, reading.coldJunction.value);
        delay(150);  // HIL tests only: wait for the next conversion (max. 100 ms)
    }
}

}  // namespace

void setup() {
    delay(2000);  // HIL tests only: give the test runner time to attach to the serial port
    bus.begin();
    delay(200);
    UNITY_BEGIN();
    RUN_TEST(test_ten_readings_at_room_temperature);
    UNITY_END();
}

void loop() {}
