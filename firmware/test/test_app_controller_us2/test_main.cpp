#include <unity.h>

#include "../fakes/app_fixture.h"
#include "screen_layout.h"

void setUp() {}
void tearDown() {}

namespace {

uint8_t raw(UiScreen screen) { return static_cast<uint8_t>(screen); }
uint8_t raw(Color color) { return static_cast<uint8_t>(color); }

}  // namespace

void test_main_menu_after_three_seconds_without_input() {
    AppFixture f;
    f.app.setup(Milliseconds{0});
    f.runUntil(2995);
    TEST_ASSERT_EQUAL_UINT8(raw(UiScreen::Welcome), raw(f.app.screen()));
    f.runUntil(3000);
    TEST_ASSERT_EQUAL_UINT8(raw(UiScreen::MainMenu), raw(f.app.screen()));
    TEST_ASSERT_NOT_NULL(f.graphics.findText("Hauptmen\xC3\xBC"));
    TEST_ASSERT_NOT_NULL(f.graphics.findText("Heizen"));
    TEST_ASSERT_NOT_NULL(f.graphics.findText("Profil"));
    TEST_ASSERT_NOT_NULL(f.graphics.findText("Setup"));
}

void test_button_on_welcome_opens_menu_immediately() {
    AppFixture f;
    f.app.setup(Milliseconds{0});
    f.runUntil(495);
    f.input.events.push_back(InputEvent::Pressed);
    f.runUntil(500);
    TEST_ASSERT_EQUAL_UINT8(raw(UiScreen::MainMenu), raw(f.app.screen()));
}

void test_rotation_on_welcome_is_ignored() {
    AppFixture f;
    f.app.setup(Milliseconds{0});
    f.input.events.push_back(InputEvent::RotateCw);
    f.runUntil(100);
    TEST_ASSERT_EQUAL_UINT8(raw(UiScreen::Welcome), raw(f.app.screen()));
    TEST_ASSERT_EQUAL_size_t(0, f.menu.cursor());
}

void test_menu_clears_content_and_keeps_status_area() {
    AppFixture f;
    f.app.setup(Milliseconds{0});
    f.runUntil(2995);
    f.graphics.clear();
    f.runUntil(3000);

    bool contentCleared = false;
    for (const FakeGraphics::Rect& rect : f.graphics.rects) {
        if (rect.y == layout::kContentTop && rect.h == layout::kContent.h) {
            contentCleared = true;
        }
    }
    TEST_ASSERT_TRUE(contentCleared);
    TEST_ASSERT_NOT_NULL(f.graphics.findText("WLAN nicht verbunden"));  // FR-010
}

void test_rotation_moves_cursor() {
    AppFixture f;
    f.app.setup(Milliseconds{0});
    f.runUntil(3000);
    f.graphics.clear();
    f.input.events.push_back(InputEvent::RotateCw);
    f.runUntil(3005);
    TEST_ASSERT_EQUAL_size_t(1, f.menu.cursor());
    TEST_ASSERT_EQUAL_UINT8(raw(Color::Accent), raw(f.graphics.findText("Profil")->bg));

    f.input.events.push_back(InputEvent::RotateCcw);
    f.input.events.push_back(InputEvent::RotateCcw);
    f.runUntil(3010);
    TEST_ASSERT_EQUAL_size_t(0, f.menu.cursor());
}

void test_selection_only_logs_and_draws_nothing() {
    AppFixture f;
    f.app.setup(Milliseconds{0});
    f.runUntil(3000);
    f.input.events.push_back(InputEvent::RotateCw);
    f.input.events.push_back(InputEvent::RotateCw);
    f.runUntil(3005);
    f.graphics.clear();

    f.input.events.push_back(InputEvent::Pressed);
    f.runUntil(3010);

    TEST_ASSERT_EQUAL_UINT8(raw(UiScreen::MainMenu), raw(f.app.screen()));
    TEST_ASSERT_EQUAL_size_t(2, f.menu.cursor());
    TEST_ASSERT_EQUAL_size_t(0, f.graphics.calls);
    const FakeLogger::Entry* entry = f.logger.find("menu select");
    TEST_ASSERT_NOT_NULL(entry);
    TEST_ASSERT_EQUAL_STRING("entry=setup action=none", entry->fields.c_str());
    TEST_ASSERT_EQUAL_STRING("ui", entry->module.c_str());
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_main_menu_after_three_seconds_without_input);
    RUN_TEST(test_button_on_welcome_opens_menu_immediately);
    RUN_TEST(test_rotation_on_welcome_is_ignored);
    RUN_TEST(test_menu_clears_content_and_keeps_status_area);
    RUN_TEST(test_rotation_moves_cursor);
    RUN_TEST(test_selection_only_logs_and_draws_nothing);
    return UNITY_END();
}
