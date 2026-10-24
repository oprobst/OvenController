#include <unity.h>

#include "../fakes/fake_graphics.h"
#include "main_menu.h"
#include "menu_screen.h"
#include "screen_layout.h"

void setUp() {}
void tearDown() {}

namespace {

void fill(MainMenu& menu) {
    menu.add({MenuEntryId::Heat, "Heizen"});
    menu.add({MenuEntryId::Profile, "Profil"});
    menu.add({MenuEntryId::Setup, "Setup"});
}

uint8_t raw(Color color) { return static_cast<uint8_t>(color); }

}  // namespace

void test_draws_header_and_entries() {
    FakeGraphics graphics;
    MainMenu menu;
    fill(menu);
    MenuScreen screen(graphics);
    screen.draw(menu);

    const FakeGraphics::Text* header = graphics.findText("Hauptmen\xC3\xBC");
    TEST_ASSERT_NOT_NULL(header);
    TEST_ASSERT_EQUAL_INT16(56, header->y);
    TEST_ASSERT_EQUAL_UINT8(raw(Color::Muted), raw(header->fg));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(FontSize::Medium),
                            static_cast<uint8_t>(header->size));

    const char* labels[] = {"Heizen", "Profil", "Setup"};
    for (int i = 0; i < 3; ++i) {
        const FakeGraphics::Text* entry = graphics.findText(labels[i]);
        TEST_ASSERT_NOT_NULL(entry);
        TEST_ASSERT_EQUAL_INT16(84 + i * 40, entry->y);
        TEST_ASSERT_EQUAL_UINT16(40, entry->h);
        TEST_ASSERT_EQUAL_UINT16(240, entry->w);
        TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(FontSize::Large),
                                static_cast<uint8_t>(entry->size));
    }
}

void test_marked_entry_uses_accent_background() {
    FakeGraphics graphics;
    MainMenu menu;
    fill(menu);
    MenuScreen screen(graphics);
    screen.draw(menu);

    const FakeGraphics::Text* marked = graphics.findText("Heizen");
    TEST_ASSERT_EQUAL_UINT8(raw(Color::Accent), raw(marked->bg));
    TEST_ASSERT_EQUAL_UINT8(raw(Color::Background), raw(marked->fg));
    const FakeGraphics::Text* other = graphics.findText("Profil");
    TEST_ASSERT_EQUAL_UINT8(raw(Color::Background), raw(other->bg));
    TEST_ASSERT_EQUAL_UINT8(raw(Color::Foreground), raw(other->fg));
}

void test_rotation_redraws_only_old_and_new_entry() {
    FakeGraphics graphics;
    MainMenu menu;
    fill(menu);
    MenuScreen screen(graphics);
    screen.draw(menu);
    graphics.clear();

    menu.moveBy(1);
    screen.update(menu);
    TEST_ASSERT_EQUAL_size_t(2, graphics.calls);
    TEST_ASSERT_EQUAL_UINT8(raw(Color::Background), raw(graphics.findText("Heizen")->bg));
    TEST_ASSERT_EQUAL_UINT8(raw(Color::Accent), raw(graphics.findText("Profil")->bg));
}

void test_update_without_cursor_change_draws_nothing() {
    FakeGraphics graphics;
    MainMenu menu;
    fill(menu);
    MenuScreen screen(graphics);
    screen.draw(menu);
    graphics.clear();
    screen.update(menu);
    TEST_ASSERT_EQUAL_size_t(0, graphics.calls);
}

void test_at_most_five_entries_visible() {
    FakeGraphics graphics;
    MainMenu menu;
    const char* labels[] = {"a", "b", "c", "d", "e", "f"};
    for (const char* label : labels) {
        menu.add({MenuEntryId::Setup, label});
    }
    MenuScreen screen(graphics);
    screen.draw(menu);
    TEST_ASSERT_NOT_NULL(graphics.findText("e"));
    TEST_ASSERT_NULL(graphics.findText("f"));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_draws_header_and_entries);
    RUN_TEST(test_marked_entry_uses_accent_background);
    RUN_TEST(test_rotation_redraws_only_old_and_new_entry);
    RUN_TEST(test_update_without_cursor_change_draws_nothing);
    RUN_TEST(test_at_most_five_entries_visible);
    return UNITY_END();
}
