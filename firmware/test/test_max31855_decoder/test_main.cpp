#include <unity.h>

#include "max31855_decoder.h"

void setUp() {}
void tearDown() {}

namespace {

/// Builds a raw word from the hot junction in 0.25 degC steps and the cold junction in
/// 0.0625 degC steps, without fault bits.
uint32_t rawWord(int32_t hotQuarterDegrees, int32_t coldSixteenthDegrees) {
    return ((static_cast<uint32_t>(hotQuarterDegrees) & 0x3FFFu) << 18) |
           ((static_cast<uint32_t>(coldSixteenthDegrees) & 0x0FFFu) << 4);
}

constexpr uint32_t kFaultBit = 1u << 16;

}  // namespace

void test_all_zero_word_means_no_response() {
    TEST_ASSERT_EQUAL_UINT16(static_cast<uint16_t>(ErrorCode::SensorNoResponse),
                             static_cast<uint16_t>(Max31855Decoder::decode(0x00000000u).status));
}

void test_all_one_word_means_no_response() {
    TEST_ASSERT_EQUAL_UINT16(static_cast<uint16_t>(ErrorCode::SensorNoResponse),
                             static_cast<uint16_t>(Max31855Decoder::decode(0xFFFFFFFFu).status));
}

void test_fault_bits_map_to_error_codes() {
    const uint32_t base = rawWord(100, 400) | kFaultBit;
    TEST_ASSERT_EQUAL_UINT16(static_cast<uint16_t>(ErrorCode::ThermocoupleOpen),
                             static_cast<uint16_t>(Max31855Decoder::decode(base | 0x1u).status));
    TEST_ASSERT_EQUAL_UINT16(static_cast<uint16_t>(ErrorCode::ThermocoupleShortGnd),
                             static_cast<uint16_t>(Max31855Decoder::decode(base | 0x2u).status));
    TEST_ASSERT_EQUAL_UINT16(static_cast<uint16_t>(ErrorCode::ThermocoupleShortVcc),
                             static_cast<uint16_t>(Max31855Decoder::decode(base | 0x4u).status));
}

void test_fault_bit_without_detail_means_no_response() {
    const uint32_t raw = rawWord(100, 400) | kFaultBit;
    TEST_ASSERT_EQUAL_UINT16(static_cast<uint16_t>(ErrorCode::SensorNoResponse),
                             static_cast<uint16_t>(Max31855Decoder::decode(raw).status));
}

void test_decodes_hot_junction_temperatures() {
    struct Case {
        int32_t quarters;
        float celsius;
    };
    const Case cases[] = {{100, 25.0f}, {4095, 1023.75f}, {-1, -0.25f}, {-1000, -250.0f}};
    for (const Case& c : cases) {
        const TemperatureReading reading = Max31855Decoder::decode(rawWord(c.quarters, 400));
        TEST_ASSERT_EQUAL_UINT16(0, static_cast<uint16_t>(reading.status));
        TEST_ASSERT_EQUAL_FLOAT(c.celsius, reading.hotJunction.value);
    }
}

void test_decodes_cold_junction_temperatures() {
    TEST_ASSERT_EQUAL_FLOAT(25.0625f,
                            Max31855Decoder::decode(rawWord(100, 401)).coldJunction.value);
    TEST_ASSERT_EQUAL_FLOAT(-0.0625f, Max31855Decoder::decode(rawWord(100, -1)).coldJunction.value);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_all_zero_word_means_no_response);
    RUN_TEST(test_all_one_word_means_no_response);
    RUN_TEST(test_fault_bits_map_to_error_codes);
    RUN_TEST(test_fault_bit_without_detail_means_no_response);
    RUN_TEST(test_decodes_hot_junction_temperatures);
    RUN_TEST(test_decodes_cold_junction_temperatures);
    return UNITY_END();
}
