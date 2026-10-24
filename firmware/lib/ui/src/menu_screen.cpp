#include "menu_screen.h"

#include "screen_layout.h"

MenuScreen::MenuScreen(IGraphics& graphics) : graphics_(graphics) {}

void MenuScreen::draw(const MainMenu& menu) {
    using namespace layout;
    graphics_.drawText(kMenuHeader.x, kMenuHeader.y, kMenuHeader.w, kMenuHeader.h,
                       "Hauptmen\xC3\xBC", FontSize::Medium, Color::Muted, Color::Background,
                       Align::Left);
    for (size_t i = 0; i < menu.count() && i < kMenuVisibleEntries; ++i) {
        drawEntry(menu, i);
    }
    shownCursor_ = menu.cursor();
}

void MenuScreen::update(const MainMenu& menu) {
    if (menu.cursor() == shownCursor_) {
        return;
    }
    const size_t previous = shownCursor_;
    shownCursor_ = menu.cursor();
    drawEntry(menu, previous);
    drawEntry(menu, shownCursor_);
}

void MenuScreen::drawEntry(const MainMenu& menu, size_t index) {
    using namespace layout;
    if (index >= menu.count() || index >= kMenuVisibleEntries) {
        return;
    }
    const bool marked = index == menu.cursor();
    const int16_t y = static_cast<int16_t>(kMenuFirstEntryY + index * kMenuEntryHeight);
    graphics_.drawText(0, y, kScreenWidth, kMenuEntryHeight, menu.entry(index).label,
                       FontSize::Large, marked ? Color::Background : Color::Foreground,
                       marked ? Color::Accent : Color::Background, Align::Left);
}
