#include <unity.h>

#include "scheduler.h"

void setUp() {}
void tearDown() {}

namespace {

void countRun(void* context, Milliseconds /*now*/) { ++*static_cast<int*>(context); }

}  // namespace

void test_runs_task_on_first_tick_and_then_with_its_period() {
    Scheduler scheduler;
    int runs = 0;
    TEST_ASSERT_TRUE(scheduler.add(countRun, &runs, 250));

    scheduler.tick(Milliseconds{0});
    TEST_ASSERT_EQUAL_INT(1, runs);
    scheduler.tick(Milliseconds{249});
    TEST_ASSERT_EQUAL_INT(1, runs);
    scheduler.tick(Milliseconds{250});
    TEST_ASSERT_EQUAL_INT(2, runs);
    scheduler.tick(Milliseconds{499});
    TEST_ASSERT_EQUAL_INT(2, runs);
    scheduler.tick(Milliseconds{500});
    TEST_ASSERT_EQUAL_INT(3, runs);
}

void test_period_zero_runs_every_tick() {
    Scheduler scheduler;
    int runs = 0;
    scheduler.add(countRun, &runs, 0);
    scheduler.tick(Milliseconds{0});
    scheduler.tick(Milliseconds{0});
    scheduler.tick(Milliseconds{1});
    TEST_ASSERT_EQUAL_INT(3, runs);
}

void test_keeps_period_across_millis_overflow() {
    Scheduler scheduler;
    int runs = 0;
    scheduler.add(countRun, &runs, 100);

    scheduler.tick(Milliseconds{0xFFFFFFC0u});
    TEST_ASSERT_EQUAL_INT(1, runs);
    scheduler.tick(Milliseconds{0x00000010u});  // 0x50 = 80 ms later
    TEST_ASSERT_EQUAL_INT(1, runs);
    scheduler.tick(Milliseconds{0x00000024u});  // 100 ms later
    TEST_ASSERT_EQUAL_INT(2, runs);
}

void test_rejects_ninth_task() {
    Scheduler scheduler;
    int runs = 0;
    for (size_t i = 0; i < Scheduler::kMaxTasks; ++i) {
        TEST_ASSERT_TRUE(scheduler.add(countRun, &runs, 10));
    }
    TEST_ASSERT_FALSE(scheduler.add(countRun, &runs, 10));
}

void test_does_not_catch_up_after_long_pause() {
    Scheduler scheduler;
    int runs = 0;
    scheduler.add(countRun, &runs, 100);
    scheduler.tick(Milliseconds{0});
    scheduler.tick(Milliseconds{1000});  // ten periods missed
    TEST_ASSERT_EQUAL_INT(2, runs);
    scheduler.tick(Milliseconds{1050});
    TEST_ASSERT_EQUAL_INT(2, runs);
    scheduler.tick(Milliseconds{1100});
    TEST_ASSERT_EQUAL_INT(3, runs);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_runs_task_on_first_tick_and_then_with_its_period);
    RUN_TEST(test_period_zero_runs_every_tick);
    RUN_TEST(test_keeps_period_across_millis_overflow);
    RUN_TEST(test_rejects_ninth_task);
    RUN_TEST(test_does_not_catch_up_after_long_pause);
    return UNITY_END();
}
