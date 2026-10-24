#include <unity.h>

#include "../fakes/fake_input.h"
#include "rotary_input.h"

void setUp() {}
void tearDown() {}

namespace {

uint8_t raw(InputEvent event) { return static_cast<uint8_t>(event); }

}  // namespace

void test_empty_queue_returns_none() {
    FakeEncoderPins pins;
    RotaryInput input(pins);
    input.poll(Milliseconds{0});
    TEST_ASSERT_EQUAL_UINT8(raw(InputEvent::None), raw(input.nextEvent()));
}

void test_detents_become_rotation_events() {
    FakeEncoderPins pins;
    RotaryInput input(pins);
    pins.detents = 2;
    input.poll(Milliseconds{0});
    TEST_ASSERT_EQUAL_UINT8(raw(InputEvent::RotateCw), raw(input.nextEvent()));
    TEST_ASSERT_EQUAL_UINT8(raw(InputEvent::RotateCw), raw(input.nextEvent()));
    TEST_ASSERT_EQUAL_UINT8(raw(InputEvent::None), raw(input.nextEvent()));

    pins.detents = -1;
    input.poll(Milliseconds{5});
    TEST_ASSERT_EQUAL_UINT8(raw(InputEvent::RotateCcw), raw(input.nextEvent()));
    TEST_ASSERT_EQUAL_UINT8(raw(InputEvent::None), raw(input.nextEvent()));
}

void test_events_are_delivered_in_order() {
    FakeEncoderPins pins;
    RotaryInput input(pins);
    uint32_t now = 0;
    for (; now < 50; now += 5) {
        input.poll(Milliseconds{now});
    }
    pins.detents = 1;
    input.poll(Milliseconds{now});
    now += 5;
    pins.pressed = true;
    for (int i = 0; i < 6; ++i, now += 5) {
        input.poll(Milliseconds{now});
    }
    pins.detents = -1;
    input.poll(Milliseconds{now});

    TEST_ASSERT_EQUAL_UINT8(raw(InputEvent::RotateCw), raw(input.nextEvent()));
    TEST_ASSERT_EQUAL_UINT8(raw(InputEvent::Pressed), raw(input.nextEvent()));
    TEST_ASSERT_EQUAL_UINT8(raw(InputEvent::RotateCcw), raw(input.nextEvent()));
    TEST_ASSERT_EQUAL_UINT8(raw(InputEvent::None), raw(input.nextEvent()));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_empty_queue_returns_none);
    RUN_TEST(test_detents_become_rotation_events);
    RUN_TEST(test_events_are_delivered_in_order);
    return UNITY_END();
}
