#include <unity.h>

#include "button_debouncer.h"

void setUp() {}
void tearDown() {}

namespace {

/// Feeds the same level every 5 ms for durationMs; returns the number of press events.
int hold(ButtonDebouncer& debouncer, bool pressed, uint32_t& nowMs, uint32_t durationMs) {
    int events = 0;
    for (uint32_t t = 0; t < durationMs; t += 5) {
        if (debouncer.update(pressed, Milliseconds{nowMs})) {
            ++events;
        }
        nowMs += 5;
    }
    return events;
}

}  // namespace

void test_stable_press_gives_exactly_one_event() {
    ButtonDebouncer debouncer(20);
    uint32_t now = 0;
    TEST_ASSERT_EQUAL_INT(0, hold(debouncer, false, now, 50));
    TEST_ASSERT_EQUAL_INT(1, hold(debouncer, true, now, 200));
}

void test_press_needs_twenty_ms_stable_level() {
    ButtonDebouncer debouncer(20);
    uint32_t now = 0;
    hold(debouncer, false, now, 50);
    TEST_ASSERT_EQUAL_INT(0, hold(debouncer, true, now, 15));
    TEST_ASSERT_EQUAL_INT(1, hold(debouncer, true, now, 10));
}

void test_bounce_shorter_than_twenty_ms_gives_no_event() {
    ButtonDebouncer debouncer(20);
    uint32_t now = 0;
    hold(debouncer, false, now, 50);
    for (int i = 0; i < 5; ++i) {
        TEST_ASSERT_EQUAL_INT(0, hold(debouncer, true, now, 10));
        TEST_ASSERT_EQUAL_INT(0, hold(debouncer, false, now, 10));
    }
}

void test_release_gives_no_event_and_next_press_counts_again() {
    ButtonDebouncer debouncer(20);
    uint32_t now = 0;
    hold(debouncer, false, now, 50);
    TEST_ASSERT_EQUAL_INT(1, hold(debouncer, true, now, 100));
    TEST_ASSERT_EQUAL_INT(0, hold(debouncer, false, now, 100));
    TEST_ASSERT_EQUAL_INT(1, hold(debouncer, true, now, 100));
}

void test_button_held_at_start_gives_no_event() {
    ButtonDebouncer debouncer(20);
    uint32_t now = 0;
    TEST_ASSERT_EQUAL_INT(0, hold(debouncer, true, now, 200));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_stable_press_gives_exactly_one_event);
    RUN_TEST(test_press_needs_twenty_ms_stable_level);
    RUN_TEST(test_bounce_shorter_than_twenty_ms_gives_no_event);
    RUN_TEST(test_release_gives_no_event_and_next_press_counts_again);
    RUN_TEST(test_button_held_at_start_gives_no_event);
    return UNITY_END();
}
