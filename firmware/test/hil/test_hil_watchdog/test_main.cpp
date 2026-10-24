// HIL test: the task watchdog restarts the controller when the main loop blocks.
//
// Run 1: the reset reason is not "task_wdt" -> start the watchdog and block without feeding it.
//        The controller must restart within about 6 s.
// Run 2: after that restart the reset reason is "task_wdt" -> test passes.
// PlatformIO's test runner waits for the result line, so both runs appear as one test.

#include <Arduino.h>
#include <unity.h>

#include <cstring>

#include "esp32_platform.h"

namespace {

Esp32SystemInfo systemInfo;
Esp32Watchdog watchdog;

void test_watchdog_restarted_controller() {
    TEST_ASSERT_EQUAL_STRING("task_wdt", systemInfo.resetReason());
}

}  // namespace

void setup() {
    if (std::strcmp(systemInfo.resetReason(), "task_wdt") != 0) {
        Serial.begin(115200);
        Serial.println("Blocking loop without feeding the watchdog; expecting a restart...");
        watchdog.start();
        const uint32_t start = millis();
        while (millis() - start < 10000) {
            // Busy wait on purpose: the watchdog must fire before this loop ends.
        }
        Serial.println("Watchdog did not fire");
    }
    delay(2000);  // HIL tests only: give the test runner time to attach to the serial port
    UNITY_BEGIN();
    RUN_TEST(test_watchdog_restarted_controller);
    UNITY_END();
}

void loop() {}
