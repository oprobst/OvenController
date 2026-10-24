#include <unity.h>

#include "status_formatter.h"

void setUp() {}
void tearDown() {}

namespace {

WallClockTime validTime() { return WallClockTime{true, 2026, 3, 7, 9, 5}; }
WallClockTime invalidTime() { return WallClockTime{false, 1970, 1, 1, 0, 0}; }
OvenTemperature validOven(float celsius) {
    return OvenTemperature{true, Celsius{celsius}, ErrorCode::None};
}

SystemStatus status(WifiState wifi, WallClockTime clock, OvenTemperature oven) {
    return SystemStatus{wifi, clock, oven};
}

void assertColor(Color expected, Color actual) {
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(expected), static_cast<uint8_t>(actual));
}

}  // namespace

void test_wifi_texts_and_colors() {
    StatusTexts t =
        StatusFormatter::format(status(WifiState::Connected, validTime(), validOven(20)));
    TEST_ASSERT_EQUAL_STRING("WLAN verbunden", t.wifi.text);
    assertColor(Color::Foreground, t.wifi.color);

    t = StatusFormatter::format(status(WifiState::Disconnected, validTime(), validOven(20)));
    TEST_ASSERT_EQUAL_STRING("WLAN nicht verbunden", t.wifi.text);
    assertColor(Color::Warning, t.wifi.color);

    t = StatusFormatter::format(status(WifiState::NotConfigured, validTime(), validOven(20)));
    TEST_ASSERT_EQUAL_STRING("WLAN nicht eingerichtet", t.wifi.text);
    assertColor(Color::Muted, t.wifi.color);
}

void test_valid_time_with_leading_zeros() {
    const StatusTexts t =
        StatusFormatter::format(status(WifiState::Connected, validTime(), validOven(20)));
    TEST_ASSERT_EQUAL_STRING("09:05", t.time.text);
    TEST_ASSERT_EQUAL_STRING("07.03.2026", t.date.text);
    assertColor(Color::Foreground, t.time.color);
    assertColor(Color::Foreground, t.date.color);
}

void test_connected_without_valid_time_shows_placeholders() {
    const StatusTexts t =
        StatusFormatter::format(status(WifiState::Connected, invalidTime(), validOven(20)));
    TEST_ASSERT_EQUAL_STRING("--:--", t.time.text);
    TEST_ASSERT_EQUAL_STRING("--.--.----", t.date.text);
    assertColor(Color::Muted, t.time.color);
    assertColor(Color::Muted, t.date.color);
}

void test_not_connected_and_never_synced_shows_nothing() {
    const StatusTexts t =
        StatusFormatter::format(status(WifiState::Disconnected, invalidTime(), validOven(20)));
    TEST_ASSERT_EQUAL_STRING("", t.time.text);
    TEST_ASSERT_EQUAL_STRING("", t.date.text);
}

void test_not_connected_but_synced_keeps_running_time() {
    const StatusTexts t =
        StatusFormatter::format(status(WifiState::Disconnected, validTime(), validOven(20)));
    TEST_ASSERT_EQUAL_STRING("09:05", t.time.text);
    TEST_ASSERT_EQUAL_STRING("07.03.2026", t.date.text);
}

void test_never_shows_date_before_2026() {
    WallClockTime old = validTime();
    old.year = 1970;
    const StatusTexts t = StatusFormatter::format(status(WifiState::Connected, old, validOven(20)));
    TEST_ASSERT_EQUAL_STRING("--:--", t.time.text);
    TEST_ASSERT_EQUAL_STRING("--.--.----", t.date.text);
}

void test_temperature_rounded_whole_degrees() {
    StatusTexts t =
        StatusFormatter::format(status(WifiState::Connected, validTime(), validOven(23.5f)));
    TEST_ASSERT_EQUAL_STRING(
        "24 \xC2\xB0"
        "C",
        t.temperature.text);
    assertColor(Color::Foreground, t.temperature.color);
    TEST_ASSERT_EQUAL_STRING("", t.faultLine.text);

    t = StatusFormatter::format(status(WifiState::Connected, validTime(), validOven(-3.4f)));
    TEST_ASSERT_EQUAL_STRING(
        "-3 \xC2\xB0"
        "C",
        t.temperature.text);
}

void test_temperature_before_first_reading() {
    const OvenTemperature unknown{false, Celsius{0}, ErrorCode::None};
    const StatusTexts t =
        StatusFormatter::format(status(WifiState::Connected, validTime(), unknown));
    TEST_ASSERT_EQUAL_STRING(
        "--- \xC2\xB0"
        "C",
        t.temperature.text);
    assertColor(Color::Muted, t.temperature.color);
    TEST_ASSERT_EQUAL_STRING("", t.faultLine.text);
}

void test_sensor_fault_shows_code_and_fault_line() {
    const OvenTemperature fault{false, Celsius{23}, ErrorCode::ThermocoupleOpen};
    const StatusTexts t = StatusFormatter::format(status(WifiState::Connected, validTime(), fault));
    TEST_ASSERT_EQUAL_STRING("E101", t.temperature.text);
    assertColor(Color::Error, t.temperature.color);
    TEST_ASSERT_EQUAL_STRING("F\xC3\xBChlerfehler E101", t.faultLine.text);
    assertColor(Color::Error, t.faultLine.color);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_wifi_texts_and_colors);
    RUN_TEST(test_valid_time_with_leading_zeros);
    RUN_TEST(test_connected_without_valid_time_shows_placeholders);
    RUN_TEST(test_not_connected_and_never_synced_shows_nothing);
    RUN_TEST(test_not_connected_but_synced_keeps_running_time);
    RUN_TEST(test_never_shows_date_before_2026);
    RUN_TEST(test_temperature_rounded_whole_degrees);
    RUN_TEST(test_temperature_before_first_reading);
    RUN_TEST(test_sensor_fault_shows_code_and_fault_line);
    return UNITY_END();
}
