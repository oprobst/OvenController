#include <unity.h>

#include "../fakes/fake_logger.h"
#include "ui_state_machine.h"

void setUp() {}
void tearDown() {}

namespace {

constexpr UiEvent kAllEvents[] = {UiEvent::DisplayReady, UiEvent::WelcomeTimeout,
                                  UiEvent::ButtonPressed, UiEvent::RotateCw, UiEvent::RotateCcw};

uint8_t raw(UiScreen screen) { return static_cast<uint8_t>(screen); }

UiStateMachine machineIn(UiScreen target, FakeLogger& logger) {
    UiStateMachine machine(logger);
    if (target != UiScreen::Boot) {
        machine.handle(UiEvent::DisplayReady);
    }
    if (target == UiScreen::MainMenu) {
        machine.handle(UiEvent::WelcomeTimeout);
    }
    logger.entries.clear();
    return machine;
}

void assertTransition(UiScreen from, UiEvent event, UiScreen to) {
    FakeLogger logger;
    UiStateMachine machine = machineIn(from, logger);
    TEST_ASSERT_TRUE(machine.handle(event));
    TEST_ASSERT_EQUAL_UINT8(raw(to), raw(machine.current()));
    TEST_ASSERT_EQUAL_size_t(0, logger.entries.size());
}

void assertRejected(UiScreen from, UiEvent event) {
    FakeLogger logger;
    UiStateMachine machine = machineIn(from, logger);
    TEST_ASSERT_FALSE(machine.handle(event));
    TEST_ASSERT_EQUAL_UINT8(raw(from), raw(machine.current()));
    TEST_ASSERT_EQUAL_size_t(1, logger.count(LogLevel::Debug, "transition rejected"));
}

}  // namespace

// --- User Story 1 ---

void test_starts_in_boot() {
    FakeLogger logger;
    UiStateMachine machine(logger);
    TEST_ASSERT_EQUAL_UINT8(raw(UiScreen::Boot), raw(machine.current()));
}

void test_boot_display_ready_goes_to_welcome() {
    assertTransition(UiScreen::Boot, UiEvent::DisplayReady, UiScreen::Welcome);
}

void test_boot_rejects_all_other_events() {
    for (UiEvent event : kAllEvents) {
        if (event != UiEvent::DisplayReady) {
            assertRejected(UiScreen::Boot, event);
        }
    }
}

void test_welcome_rejects_display_ready() {
    assertRejected(UiScreen::Welcome, UiEvent::DisplayReady);
}

void test_rejection_log_names_screen_and_event() {
    FakeLogger logger;
    UiStateMachine machine = machineIn(UiScreen::Welcome, logger);
    machine.handle(UiEvent::RotateCw);
    TEST_ASSERT_EQUAL_STRING("screen=welcome event=rotate_cw",
                             logger.find("transition rejected")->fields.c_str());
}

// --- User Story 2 ---

void test_welcome_timeout_goes_to_main_menu() {
    assertTransition(UiScreen::Welcome, UiEvent::WelcomeTimeout, UiScreen::MainMenu);
}

void test_welcome_button_goes_to_main_menu() {
    assertTransition(UiScreen::Welcome, UiEvent::ButtonPressed, UiScreen::MainMenu);
}

void test_welcome_rejects_rotation() {
    assertRejected(UiScreen::Welcome, UiEvent::RotateCw);
    assertRejected(UiScreen::Welcome, UiEvent::RotateCcw);
}

void test_main_menu_accepts_rotation_and_button() {
    assertTransition(UiScreen::MainMenu, UiEvent::RotateCw, UiScreen::MainMenu);
    assertTransition(UiScreen::MainMenu, UiEvent::RotateCcw, UiScreen::MainMenu);
    assertTransition(UiScreen::MainMenu, UiEvent::ButtonPressed, UiScreen::MainMenu);
}

void test_main_menu_rejects_timeout_and_display_ready() {
    assertRejected(UiScreen::MainMenu, UiEvent::WelcomeTimeout);
    assertRejected(UiScreen::MainMenu, UiEvent::DisplayReady);
}

void test_transition_table_matches_data_model() {
    // Every row of the table in data-model.md, and nothing else.
    TEST_ASSERT_EQUAL_size_t(6, UiStateMachine::transitionCount());
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_starts_in_boot);
    RUN_TEST(test_boot_display_ready_goes_to_welcome);
    RUN_TEST(test_boot_rejects_all_other_events);
    RUN_TEST(test_welcome_rejects_display_ready);
    RUN_TEST(test_rejection_log_names_screen_and_event);
    RUN_TEST(test_welcome_timeout_goes_to_main_menu);
    RUN_TEST(test_welcome_button_goes_to_main_menu);
    RUN_TEST(test_welcome_rejects_rotation);
    RUN_TEST(test_main_menu_accepts_rotation_and_button);
    RUN_TEST(test_main_menu_rejects_timeout_and_display_ready);
    RUN_TEST(test_transition_table_matches_data_model);
    return UNITY_END();
}
