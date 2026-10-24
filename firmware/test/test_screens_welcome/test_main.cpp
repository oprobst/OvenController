#include <unity.h>

#include "../fakes/fake_graphics.h"
#include "screen_layout.h"
#include "status_bar.h"
#include "status_formatter.h"
#include "welcome_screen.h"

void setUp() {}
void tearDown() {}

namespace {

SystemStatus okStatus() {
    return SystemStatus{WifiState::Connected, WallClockTime{true, 2026, 10, 24, 14, 32},
                        OvenTemperature{true, Celsius{23.0f}, ErrorCode::None}};
}

void assertBox(const layout::Box& box, const FakeGraphics::Text* text) {
    TEST_ASSERT_NOT_NULL(text);
    TEST_ASSERT_EQUAL_INT16(box.x, text->x);
    TEST_ASSERT_EQUAL_INT16(box.y, text->y);
    TEST_ASSERT_EQUAL_UINT16(box.w, text->w);
    TEST_ASSERT_EQUAL_UINT16(box.h, text->h);
}

}  // namespace

void test_status_bar_layout_two_rows_with_separator() {
    FakeGraphics graphics;
    StatusBar bar(graphics);
    bar.draw(StatusFormatter::format(okStatus()));

    assertBox(layout::kStatusWifi, graphics.findText("WLAN verbunden"));
    assertBox(layout::kStatusTime, graphics.findText("14:32"));
    assertBox(layout::kStatusDate, graphics.findText("24.10.2026"));
    assertBox(layout::kStatusTemperature, graphics.findText("23 \xC2\xB0"
                                                            "C"));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Align::Right),
                            static_cast<uint8_t>(graphics.findText("14:32")->align));

    bool separatorDrawn = false;
    for (const FakeGraphics::Rect& rect : graphics.rects) {
        if (rect.y == 48 && rect.h == 1 && rect.w == 240) {
            separatorDrawn = true;
        }
    }
    TEST_ASSERT_TRUE(separatorDrawn);
    TEST_ASSERT_EQUAL_INT16(0, layout::kStatusWifi.y);
    TEST_ASSERT_EQUAL_INT16(24, layout::kStatusDate.y);
}

void test_status_bar_redraws_only_changed_regions() {
    FakeGraphics graphics;
    StatusBar bar(graphics);
    SystemStatus status = okStatus();
    bar.draw(StatusFormatter::format(status));
    graphics.clear();

    bar.draw(StatusFormatter::format(status));
    TEST_ASSERT_EQUAL_size_t(0, graphics.calls);

    status.oven.temperature = Celsius{24.0f};
    bar.draw(StatusFormatter::format(status));
    TEST_ASSERT_EQUAL_size_t(1, graphics.calls);
    TEST_ASSERT_NOT_NULL(
        graphics.findText("24 \xC2\xB0"
                          "C"));
}

void test_status_bar_shows_and_clears_fault_line() {
    FakeGraphics graphics;
    StatusBar bar(graphics);
    SystemStatus status = okStatus();
    status.oven = OvenTemperature{false, Celsius{23.0f}, ErrorCode::ThermocoupleOpen};
    bar.draw(StatusFormatter::format(status));
    assertBox(layout::kFaultLine, graphics.findText("F\xC3\xBChlerfehler E101"));
    TEST_ASSERT_EQUAL_INT16(292, layout::kFaultLine.y);

    graphics.clear();
    bar.draw(StatusFormatter::format(okStatus()));
    const FakeGraphics::Text* cleared =
        graphics.lastTextAt(layout::kFaultLine.x, layout::kFaultLine.y);
    TEST_ASSERT_NOT_NULL(cleared);
    TEST_ASSERT_EQUAL_STRING("", cleared->text.c_str());
}

void test_status_bar_invalidate_forces_full_redraw() {
    FakeGraphics graphics;
    StatusBar bar(graphics);
    const StatusTexts texts = StatusFormatter::format(okStatus());
    bar.draw(texts);
    graphics.clear();
    bar.invalidate();
    bar.draw(texts);
    TEST_ASSERT_NOT_NULL(graphics.findText("WLAN verbunden"));
    TEST_ASSERT_NOT_NULL(graphics.findText("14:32"));
}

void test_welcome_screen_texts() {
    FakeGraphics graphics;
    WelcomeScreen screen(graphics);
    screen.draw(FirmwareIdentity{"0.1.0", "DEV"});

    const FakeGraphics::Text* hello = graphics.findText("Willkommen!");
    TEST_ASSERT_NOT_NULL(hello);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(FontSize::Large),
                            static_cast<uint8_t>(hello->size));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Align::Center),
                            static_cast<uint8_t>(hello->align));

    const FakeGraphics::Text* title = graphics.findText("Ofen-Controller");
    TEST_ASSERT_NOT_NULL(title);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(FontSize::Medium),
                            static_cast<uint8_t>(title->size));

    const FakeGraphics::Text* firmware = graphics.findText("Firmware 0.1.0");
    const FakeGraphics::Text* pcb = graphics.findText("PCB REV DEV");
    TEST_ASSERT_NOT_NULL(firmware);
    TEST_ASSERT_NOT_NULL(pcb);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Color::Muted), static_cast<uint8_t>(firmware->fg));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(FontSize::Small), static_cast<uint8_t>(pcb->size));
    TEST_ASSERT_TRUE(hello->y >= layout::kContentTop);
    TEST_ASSERT_TRUE(pcb->y + pcb->h <= layout::kFaultLine.y);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_status_bar_layout_two_rows_with_separator);
    RUN_TEST(test_status_bar_redraws_only_changed_regions);
    RUN_TEST(test_status_bar_shows_and_clears_fault_line);
    RUN_TEST(test_status_bar_invalidate_forces_full_redraw);
    RUN_TEST(test_welcome_screen_texts);
    return UNITY_END();
}
