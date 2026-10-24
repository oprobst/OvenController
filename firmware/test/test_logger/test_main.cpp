#include <unity.h>

#include <string>

#include "../fakes/fake_clock.h"
#include "../fakes/fake_serial_out.h"
#include "logger.h"

void setUp() {}
void tearDown() {}

void test_writes_formatted_line_with_clock_time() {
    FakeSerialOut out;
    FakeClock clock;
    clock.current = Milliseconds{42};
    Logger logger(out, clock, LogLevel::Info);

    logger.log(LogLevel::Info, "sys", "boot", "fw=0.1.0");

    TEST_ASSERT_EQUAL_size_t(1, out.writes.size());
    TEST_ASSERT_EQUAL_STRING("t=42 lvl=INFO mod=sys msg=\"boot\" fw=0.1.0\n",
                             out.writes[0].c_str());
}

void test_filters_levels_above_configured_level() {
    FakeSerialOut out;
    FakeClock clock;
    Logger logger(out, clock, LogLevel::Warn);

    logger.log(LogLevel::Info, "sys", "info");
    logger.log(LogLevel::Debug, "sys", "debug");
    logger.log(LogLevel::Warn, "sys", "warn");
    logger.log(LogLevel::Error, "sys", "error");

    TEST_ASSERT_EQUAL_size_t(2, out.writes.size());
}

void test_counts_dropped_lines_and_reports_them_on_next_success() {
    FakeSerialOut out;
    FakeClock clock;
    Logger logger(out, clock, LogLevel::Debug);

    out.capacity = 0;  // buffer full: log() must not block, line is dropped
    logger.log(LogLevel::Info, "sys", "lost 1");
    logger.log(LogLevel::Info, "sys", "lost 2");
    TEST_ASSERT_EQUAL_UINT32(2, logger.droppedCount());
    TEST_ASSERT_EQUAL_size_t(0, out.writes.size());

    out.capacity = 4096;
    clock.current = Milliseconds{60000};
    logger.log(LogLevel::Info, "sys", "back");

    TEST_ASSERT_EQUAL_size_t(2, out.writes.size());
    TEST_ASSERT_EQUAL_STRING("t=60000 lvl=WARN mod=log msg=\"dropped\" count=2\n",
                             out.writes[0].c_str());
    TEST_ASSERT_EQUAL_STRING("t=60000 lvl=INFO mod=sys msg=\"back\"\n", out.writes[1].c_str());
    TEST_ASSERT_EQUAL_UINT32(0, logger.droppedCount());
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_writes_formatted_line_with_clock_time);
    RUN_TEST(test_filters_levels_above_configured_level);
    RUN_TEST(test_counts_dropped_lines_and_reports_them_on_next_success);
    return UNITY_END();
}
