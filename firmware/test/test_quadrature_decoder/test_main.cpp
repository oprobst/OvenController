#include <unity.h>

#include "quadrature_decoder.h"

void setUp() {}
void tearDown() {}

namespace {

// Gray code sequence of (A, B) for clockwise rotation: 00 -> 01 -> 11 -> 10 -> 00.
constexpr uint8_t kCwSequence[] = {0b00, 0b01, 0b11, 0b10, 0b00};

}  // namespace

void test_all_sixteen_transitions() {
    // Expected steps indexed by (prev << 2) | curr.
    const int8_t expected[16] = {0, +1, -1, 0, -1, 0, 0, +1, +1, 0, 0, -1, 0, -1, +1, 0};
    for (uint8_t prev = 0; prev < 4; ++prev) {
        for (uint8_t curr = 0; curr < 4; ++curr) {
            TEST_ASSERT_EQUAL_INT8(expected[(prev << 2) | curr],
                                   QuadratureDecoder::step(prev, curr));
        }
    }
}

void test_unchanged_and_double_jumps_give_zero() {
    TEST_ASSERT_EQUAL_INT8(0, QuadratureDecoder::step(0b00, 0b00));
    TEST_ASSERT_EQUAL_INT8(0, QuadratureDecoder::step(0b00, 0b11));
    TEST_ASSERT_EQUAL_INT8(0, QuadratureDecoder::step(0b01, 0b10));
}

void test_full_detent_clockwise_is_plus_four() {
    int sum = 0;
    for (size_t i = 1; i < sizeof(kCwSequence); ++i) {
        sum += QuadratureDecoder::step(kCwSequence[i - 1], kCwSequence[i]);
    }
    TEST_ASSERT_EQUAL_INT(4, sum);
}

void test_full_detent_counter_clockwise_is_minus_four() {
    int sum = 0;
    for (size_t i = sizeof(kCwSequence) - 1; i > 0; --i) {
        sum += QuadratureDecoder::step(kCwSequence[i], kCwSequence[i - 1]);
    }
    TEST_ASSERT_EQUAL_INT(-4, sum);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_all_sixteen_transitions);
    RUN_TEST(test_unchanged_and_double_jumps_give_zero);
    RUN_TEST(test_full_detent_clockwise_is_plus_four);
    RUN_TEST(test_full_detent_counter_clockwise_is_minus_four);
    return UNITY_END();
}
