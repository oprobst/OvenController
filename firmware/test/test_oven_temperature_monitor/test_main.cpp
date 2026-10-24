#include <unity.h>

#include "../fakes/fake_logger.h"
#include "../fakes/fake_thermocouple_bus.h"
#include "oven_temperature_monitor.h"

namespace {

FakeThermocoupleBus* bus;
FakeLogger* logger;
OvenTemperatureMonitor* monitor;
uint32_t nowMs;

/// Queues one raw word and samples it 250 ms after the previous sample.
void sampleRaw(uint32_t raw) {
    bus->queue.push_back(raw);
    monitor->sample(Milliseconds{nowMs});
    nowMs += 250;
}

void sampleValid(int32_t celsius) { sampleRaw(FakeThermocoupleBus::valid(celsius)); }

uint16_t errorOf(const OvenTemperature& t) { return static_cast<uint16_t>(t.error); }

}  // namespace

void setUp() {
    bus = new FakeThermocoupleBus();
    logger = new FakeLogger();
    monitor = new OvenTemperatureMonitor(*bus, *logger);
    nowMs = 0;
}

void tearDown() {
    delete monitor;
    delete logger;
    delete bus;
}

void test_unknown_before_first_valid_reading() {
    const OvenTemperature initial = monitor->current();
    TEST_ASSERT_FALSE(initial.valid);
    TEST_ASSERT_EQUAL_UINT16(0, errorOf(initial));

    sampleValid(23);  // one good reading is not enough
    TEST_ASSERT_FALSE(monitor->current().valid);
    TEST_ASSERT_EQUAL_UINT16(0, errorOf(monitor->current()));
}

void test_two_valid_readings_in_a_row_make_temperature_valid() {
    sampleValid(23);
    sampleValid(24);
    const OvenTemperature t = monitor->current();
    TEST_ASSERT_TRUE(t.valid);
    TEST_ASSERT_EQUAL_FLOAT(24.0f, t.temperature.value);
    TEST_ASSERT_EQUAL_UINT16(0, errorOf(t));

    sampleValid(30);  // further valid readings update the value immediately
    TEST_ASSERT_EQUAL_FLOAT(30.0f, monitor->current().temperature.value);
}

void test_single_fault_reading_changes_nothing() {
    sampleValid(23);
    sampleValid(23);
    sampleRaw(FakeThermocoupleBus::fault(0x1));
    TEST_ASSERT_TRUE(monitor->current().valid);
    TEST_ASSERT_EQUAL_UINT16(0, errorOf(monitor->current()));
}

void test_two_fault_readings_in_a_row_set_error_within_500_ms() {
    sampleValid(23);
    sampleValid(23);
    const uint32_t firstFaultAt = nowMs;
    sampleRaw(FakeThermocoupleBus::fault(0x1));
    sampleRaw(FakeThermocoupleBus::fault(0x1));
    const uint32_t detectedAt = nowMs - 250;
    TEST_ASSERT_FALSE(monitor->current().valid);
    TEST_ASSERT_EQUAL_UINT16(101, errorOf(monitor->current()));
    TEST_ASSERT_TRUE(detectedAt - firstFaultAt <= 500);
}

void test_error_code_is_that_of_last_fault_reading() {
    sampleRaw(FakeThermocoupleBus::fault(0x2));
    sampleRaw(FakeThermocoupleBus::fault(0x4));
    TEST_ASSERT_EQUAL_UINT16(103, errorOf(monitor->current()));
}

void test_out_of_range_readings_are_implausible() {
    sampleValid(-21);
    sampleValid(-21);
    TEST_ASSERT_EQUAL_UINT16(104, errorOf(monitor->current()));

    sampleValid(25);
    sampleValid(25);
    TEST_ASSERT_TRUE(monitor->current().valid);

    sampleValid(351);
    sampleValid(351);
    TEST_ASSERT_EQUAL_UINT16(104, errorOf(monitor->current()));
}

void test_range_limits_are_plausible() {
    sampleValid(-20);
    sampleValid(-20);
    TEST_ASSERT_TRUE(monitor->current().valid);
    sampleValid(350);
    sampleValid(350);
    TEST_ASSERT_TRUE(monitor->current().valid);
}

void test_no_accepted_value_for_1000_ms_means_no_response() {
    // Alternating readings never reach the debounce threshold in either direction.
    sampleValid(23);
    sampleRaw(FakeThermocoupleBus::fault(0x1));
    sampleValid(23);
    sampleRaw(FakeThermocoupleBus::fault(0x1));
    TEST_ASSERT_EQUAL_UINT16(0, errorOf(monitor->current()));
    sampleValid(23);  // t = 1000 ms since the first sample
    TEST_ASSERT_FALSE(monitor->current().valid);
    TEST_ASSERT_EQUAL_UINT16(105, errorOf(monitor->current()));
}

void test_recovers_after_two_valid_readings() {
    sampleRaw(FakeThermocoupleBus::fault(0x1));
    sampleRaw(FakeThermocoupleBus::fault(0x1));
    TEST_ASSERT_EQUAL_UINT16(101, errorOf(monitor->current()));
    sampleValid(22);
    TEST_ASSERT_EQUAL_UINT16(101, errorOf(monitor->current()));
    sampleValid(22);
    TEST_ASSERT_TRUE(monitor->current().valid);
    TEST_ASSERT_EQUAL_UINT16(0, errorOf(monitor->current()));
}

void test_logs_each_error_change_exactly_once() {
    sampleValid(23);
    sampleValid(23);
    TEST_ASSERT_EQUAL_size_t(0, logger->entries.size());

    sampleRaw(FakeThermocoupleBus::fault(0x1));
    sampleRaw(FakeThermocoupleBus::fault(0x1));
    sampleRaw(FakeThermocoupleBus::fault(0x1));
    TEST_ASSERT_EQUAL_size_t(1, logger->count(LogLevel::Warn, "sensor fault"));
    TEST_ASSERT_EQUAL_STRING("code=E101", logger->find("sensor fault")->fields.c_str());
    TEST_ASSERT_EQUAL_STRING("temp", logger->find("sensor fault")->module.c_str());

    sampleValid(23);
    sampleValid(23);
    sampleValid(23);
    TEST_ASSERT_EQUAL_size_t(1, logger->count(LogLevel::Info, "sensor ok"));
    TEST_ASSERT_EQUAL_STRING("temp_c=23.00 cj_c=25.00", logger->find("sensor ok")->fields.c_str());
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_unknown_before_first_valid_reading);
    RUN_TEST(test_two_valid_readings_in_a_row_make_temperature_valid);
    RUN_TEST(test_single_fault_reading_changes_nothing);
    RUN_TEST(test_two_fault_readings_in_a_row_set_error_within_500_ms);
    RUN_TEST(test_error_code_is_that_of_last_fault_reading);
    RUN_TEST(test_out_of_range_readings_are_implausible);
    RUN_TEST(test_range_limits_are_plausible);
    RUN_TEST(test_no_accepted_value_for_1000_ms_means_no_response);
    RUN_TEST(test_recovers_after_two_valid_readings);
    RUN_TEST(test_logs_each_error_change_exactly_once);
    return UNITY_END();
}
