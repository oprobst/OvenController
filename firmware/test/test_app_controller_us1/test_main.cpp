#include <unity.h>

#include "../fakes/app_fixture.h"

void setUp() {}
void tearDown() {}

void test_welcome_is_drawn_before_network_starts() {
    AppFixture f;
    bool welcomeVisibleAtBegin = false;
    f.network.onBegin = [&f, &welcomeVisibleAtBegin] {
        welcomeVisibleAtBegin = f.graphics.findText("Willkommen!") != nullptr &&
                                f.graphics.findText("Firmware 0.1.0") != nullptr &&
                                f.graphics.findText("PCB REV DEV") != nullptr;
    };
    f.app.setup(Milliseconds{0});
    TEST_ASSERT_EQUAL_INT(1, f.network.begins);
    TEST_ASSERT_EQUAL_INT(1, f.wallClock.begins);
    TEST_ASSERT_TRUE(welcomeVisibleAtBegin);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(UiScreen::Welcome),
                            static_cast<uint8_t>(f.app.screen()));
}

void test_status_area_shown_with_welcome() {
    AppFixture f;
    f.network.current = WifiState::NotConfigured;
    f.app.setup(Milliseconds{0});
    TEST_ASSERT_NOT_NULL(f.graphics.findText("WLAN nicht eingerichtet"));
    TEST_ASSERT_NOT_NULL(
        f.graphics.findText("23 \xC2\xB0"
                            "C"));
}

void test_status_follows_changes_within_one_second() {
    AppFixture f;
    f.app.setup(Milliseconds{0});
    f.runUntil(100);

    f.network.current = WifiState::Connected;
    f.wallClock.current = WallClockTime{true, 2026, 10, 24, 14, 32};
    f.temperature.state = OvenTemperature{true, Celsius{87.4f}, ErrorCode::None};
    f.runUntil(1100);

    TEST_ASSERT_NOT_NULL(f.graphics.findText("WLAN verbunden"));
    TEST_ASSERT_NOT_NULL(f.graphics.findText("14:32"));
    TEST_ASSERT_NOT_NULL(f.graphics.findText("24.10.2026"));
    TEST_ASSERT_NOT_NULL(
        f.graphics.findText("87 \xC2\xB0"
                            "C"));
}

void test_sensor_fault_is_displayed() {
    AppFixture f;
    f.app.setup(Milliseconds{0});
    f.temperature.state = OvenTemperature{false, Celsius{23.0f}, ErrorCode::ThermocoupleOpen};
    f.runUntil(200);
    TEST_ASSERT_NOT_NULL(f.graphics.findText("E101"));
    TEST_ASSERT_NOT_NULL(f.graphics.findText("F\xC3\xBChlerfehler E101"));
}

void test_periodic_tasks_and_watchdog() {
    AppFixture f;
    f.app.setup(Milliseconds{0});
    f.runUntil(1000);  // 201 ticks
    TEST_ASSERT_EQUAL_INT(201, f.watchdog.feeds);
    TEST_ASSERT_EQUAL_INT(5, f.temperature.samples);  // 0, 250, 500, 750, 1000
    TEST_ASSERT_EQUAL_INT(2, f.network.polls);        // 0, 1000
    TEST_ASSERT_EQUAL_INT(2, f.wallClock.polls);
    TEST_ASSERT_EQUAL_INT(201, f.input.polls);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_welcome_is_drawn_before_network_starts);
    RUN_TEST(test_status_area_shown_with_welcome);
    RUN_TEST(test_status_follows_changes_within_one_second);
    RUN_TEST(test_sensor_fault_is_displayed);
    RUN_TEST(test_periodic_tasks_and_watchdog);
    return UNITY_END();
}
