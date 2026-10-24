#pragma once

#include <string>
#include <vector>

#include "i_graphics.h"

/// Records all drawing calls with their parameters.
class FakeGraphics final : public IGraphics {
public:
    struct Rect {
        int16_t x;
        int16_t y;
        uint16_t w;
        uint16_t h;
        Color color;
    };

    struct Text {
        int16_t x;
        int16_t y;
        uint16_t w;
        uint16_t h;
        std::string text;
        FontSize size;
        Color fg;
        Color bg;
        Align align;
    };

    uint16_t width() const override { return 240; }
    uint16_t height() const override { return 320; }

    void fillRect(int16_t x, int16_t y, uint16_t w, uint16_t h, Color color) override {
        rects.push_back({x, y, w, h, color});
        ++calls;
    }

    void drawText(int16_t x, int16_t y, uint16_t w, uint16_t h, const char* text, FontSize size,
                  Color fg, Color bg, Align align) override {
        texts.push_back({x, y, w, h, text, size, fg, bg, align});
        ++calls;
    }

    const Text* findText(const std::string& text) const {
        for (const Text& entry : texts) {
            if (entry.text == text) {
                return &entry;
            }
        }
        return nullptr;
    }

    const Text* lastTextAt(int16_t x, int16_t y) const {
        const Text* found = nullptr;
        for (const Text& entry : texts) {
            if (entry.x == x && entry.y == y) {
                found = &entry;
            }
        }
        return found;
    }

    void clear() {
        rects.clear();
        texts.clear();
        calls = 0;
    }

    std::vector<Rect> rects;
    std::vector<Text> texts;
    size_t calls = 0;
};
