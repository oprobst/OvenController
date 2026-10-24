#include <unity.h>

#include "units.h"

void setUp() {}
void tearDown() {}

void test_celsius_rounds_half_away_from_zero() {
    TEST_ASSERT_EQUAL_INT32(23, (Celsius{23.49f}.roundedWholeDegrees()));
    TEST_ASSERT_EQUAL_INT32(24, (Celsius{23.5f}.roundedWholeDegrees()));
    TEST_ASSERT_EQUAL_INT32(-1, (Celsius{-0.5f}.roundedWholeDegrees()));
    TEST_ASSERT_EQUAL_INT32(0, (Celsius{-0.49f}.roundedWholeDegrees()));
    TEST_ASSERT_EQUAL_INT32(250, (Celsius{249.75f}.roundedWholeDegrees()));
}

void test_milliseconds_elapsed_without_overflow() {
    const Milliseconds earlier{1000};
    const Milliseconds later{1250};
    TEST_ASSERT_EQUAL_UINT32(250, later.elapsedSince(earlier));
}

void test_milliseconds_elapsed_across_overflow() {
    const Milliseconds earlier{0xFFFFFF00u};
    const Milliseconds later{0x00000010u};
    TEST_ASSERT_EQUAL_UINT32(0x110, later.elapsedSince(earlier));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_celsius_rounds_half_away_from_zero);
    RUN_TEST(test_milliseconds_elapsed_without_overflow);
    RUN_TEST(test_milliseconds_elapsed_across_overflow);
    return UNITY_END();
}
