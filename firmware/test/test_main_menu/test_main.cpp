#include <unity.h>

#include "app_config.h"
#include "main_menu.h"

void setUp() {}
void tearDown() {}

namespace {

void fill(MainMenu& menu) {
    menu.add({MenuEntryId::Heat, "Heizen"});
    menu.add({MenuEntryId::Profile, "Profil"});
    menu.add({MenuEntryId::Setup, "Setup"});
}

uint8_t raw(MenuEntryId id) { return static_cast<uint8_t>(id); }

}  // namespace

void test_entries_in_order() {
    MainMenu menu;
    fill(menu);
    TEST_ASSERT_EQUAL_size_t(3, menu.count());
    TEST_ASSERT_EQUAL_STRING("Heizen", menu.entry(0).label);
    TEST_ASSERT_EQUAL_STRING("Profil", menu.entry(1).label);
    TEST_ASSERT_EQUAL_STRING("Setup", menu.entry(2).label);
}

void test_cursor_starts_at_first_entry() {
    MainMenu menu;
    fill(menu);
    TEST_ASSERT_EQUAL_size_t(0, menu.cursor());
}

void test_no_wrap_around_at_start_and_end() {
    MainMenu menu;
    fill(menu);
    menu.moveBy(-1);
    TEST_ASSERT_EQUAL_size_t(0, menu.cursor());
    menu.moveBy(1);
    menu.moveBy(1);
    TEST_ASSERT_EQUAL_size_t(2, menu.cursor());
    menu.moveBy(1);
    TEST_ASSERT_EQUAL_size_t(2, menu.cursor());
    menu.moveBy(-5);
    TEST_ASSERT_EQUAL_size_t(0, menu.cursor());
}

void test_select_returns_marked_entry_and_keeps_cursor() {
    MainMenu menu;
    fill(menu);
    menu.moveBy(1);
    TEST_ASSERT_EQUAL_UINT8(raw(MenuEntryId::Profile), raw(menu.select()));
    TEST_ASSERT_EQUAL_size_t(1, menu.cursor());
}

void test_rejects_more_than_max_entries() {
    MainMenu menu;
    for (size_t i = 0; i < config::kMaxMenuEntries; ++i) {
        TEST_ASSERT_TRUE(menu.add({MenuEntryId::Setup, "x"}));
    }
    TEST_ASSERT_FALSE(menu.add({MenuEntryId::Setup, "x"}));
    TEST_ASSERT_EQUAL_size_t(config::kMaxMenuEntries, menu.count());
}

void test_log_names() {
    TEST_ASSERT_EQUAL_STRING("heat", MainMenu::logName(MenuEntryId::Heat));
    TEST_ASSERT_EQUAL_STRING("profile", MainMenu::logName(MenuEntryId::Profile));
    TEST_ASSERT_EQUAL_STRING("setup", MainMenu::logName(MenuEntryId::Setup));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_entries_in_order);
    RUN_TEST(test_cursor_starts_at_first_entry);
    RUN_TEST(test_no_wrap_around_at_start_and_end);
    RUN_TEST(test_select_returns_marked_entry_and_keeps_cursor);
    RUN_TEST(test_rejects_more_than_max_entries);
    RUN_TEST(test_log_names);
    return UNITY_END();
}
