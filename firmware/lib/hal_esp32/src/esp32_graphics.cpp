#include "esp32_graphics.h"

#include <algorithm>

#include "board/board.h"

namespace {

constexpr int32_t kTextPaddingPx = 4;

uint16_t toRgb565(Color color) {
    switch (color) {
        case Color::Background:
            return 0x0841;
        case Color::Foreground:
            return 0xFFFF;
        case Color::Accent:
            return 0x04DF;
        case Color::Warning:
            return 0xFFE0;
        case Color::Error:
            return 0xF800;
        case Color::Muted:
            return 0x8410;
    }
    return 0xFFFF;
}

const lgfx::GFXfont& fontFor(FontSize size) {
    switch (size) {
        case FontSize::Small:
            return lgfx::fonts::DejaVu12;
        case FontSize::Medium:
            return lgfx::fonts::DejaVu18;
        case FontSize::Large:
            return lgfx::fonts::DejaVu24;
    }
    return lgfx::fonts::DejaVu18;
}

const lgfx::GFXglyph& glyphFor(const lgfx::GFXfont& font, char c) {
    uint16_t code = static_cast<uint8_t>(c);
    if (code < font.first || code > font.last) {
        code = '?';
    }
    return font.glyph[code - font.first];
}

int32_t capHeight(const lgfx::GFXfont& font) { return -glyphFor(font, 'H').yOffset; }

/// Next code point of a UTF-8 string (only 1- and 2-byte sequences); advances text.
uint16_t nextCodePoint(const char*& text) {
    const uint8_t lead = static_cast<uint8_t>(*text++);
    if (lead < 0x80) {
        return lead;
    }
    if ((lead & 0xE0) == 0xC0 && (static_cast<uint8_t>(*text) & 0xC0) == 0x80) {
        const uint8_t trail = static_cast<uint8_t>(*text++);
        return static_cast<uint16_t>(((lead & 0x1F) << 6) | (trail & 0x3F));
    }
    // Skip continuation bytes of unsupported longer sequences.
    while ((static_cast<uint8_t>(*text) & 0xC0) == 0x80) {
        ++text;
    }
    return '?';
}

/// ASCII base letter of an umlaut, or 0 if the code point is none.
char umlautBase(uint16_t codePoint) {
    switch (codePoint) {
        case 0xE4:
            return 'a';
        case 0xF6:
            return 'o';
        case 0xFC:
            return 'u';
        case 0xC4:
            return 'A';
        case 0xD6:
            return 'O';
        case 0xDC:
            return 'U';
        default:
            return 0;
    }
}

constexpr uint16_t kSharpS = 0xDF;
constexpr uint16_t kDegreeSign = 0xB0;

}  // namespace

Esp32Graphics::Device::Device() {
    {
        auto cfg = bus_.config();
        cfg.spi_host = VSPI_HOST;
        cfg.spi_mode = 0;
        cfg.freq_write = board::kTftSpiHz;
        cfg.freq_read = 16000000;
        cfg.spi_3wire = false;
        cfg.use_lock = true;
        cfg.dma_channel = SPI_DMA_CH_AUTO;
        cfg.pin_sclk = board::kPinSpiSck;
        cfg.pin_mosi = board::kPinSpiMosi;
        cfg.pin_miso = -1;
        cfg.pin_dc = board::kPinTftDc;
        bus_.config(cfg);
        panel_.setBus(&bus_);
    }
    {
        auto cfg = panel_.config();
        cfg.pin_cs = board::kPinTftCs;
        cfg.pin_rst = board::kPinTftRst;
        cfg.pin_busy = -1;
        cfg.panel_width = board::kTftWidthPx;
        cfg.panel_height = board::kTftHeightPx;
        cfg.offset_x = 0;
        cfg.offset_y = 0;
        cfg.offset_rotation = 0;
        cfg.readable = false;
        cfg.invert = true;  // ST7789 panels usually need inversion for correct colors
        cfg.rgb_order = false;
        cfg.dlen_16bit = false;
        cfg.bus_shared = false;
        panel_.config(cfg);
    }
    {
        auto cfg = light_.config();
        cfg.pin_bl = board::kPinTftBacklight;
        cfg.invert = false;
        cfg.freq = 44100;
        cfg.pwm_channel = 7;
        light_.config(cfg);
        panel_.setLight(&light_);
    }
    setPanel(&panel_);
}

Esp32Graphics::Esp32Graphics() = default;

void Esp32Graphics::begin() {
    lcd_.init();
    lcd_.setRotation(0);
    lcd_.setBrightness(255);
    lcd_.fillScreen(toRgb565(Color::Background));
}

uint16_t Esp32Graphics::width() const { return board::kTftWidthPx; }

uint16_t Esp32Graphics::height() const { return board::kTftHeightPx; }

void Esp32Graphics::fillRect(int16_t x, int16_t y, uint16_t w, uint16_t h, Color color) {
    lcd_.fillRect(x, y, w, h, toRgb565(color));
}

void Esp32Graphics::drawText(int16_t x, int16_t y, uint16_t w, uint16_t h, const char* text,
                             FontSize size, Color fg, Color bg, Align align) {
    lcd_.fillRect(x, y, w, h, toRgb565(bg));
    if (text == nullptr || text[0] == '\0') {
        return;
    }
    const lgfx::GFXfont& font = fontFor(size);
    const uint16_t color = toRgb565(fg);
    const int32_t textWidth = renderText(0, 0, text, font, color, false);

    int32_t startX = x + kTextPaddingPx;
    if (align == Align::Center) {
        startX = x + (static_cast<int32_t>(w) - textWidth) / 2;
    } else if (align == Align::Right) {
        startX = x + static_cast<int32_t>(w) - kTextPaddingPx - textWidth;
    }
    const int32_t baseline = y + (static_cast<int32_t>(h) + capHeight(font)) / 2;

    lcd_.setClipRect(x, y, w, h);
    renderText(startX, baseline, text, font, color, true);
    lcd_.clearClipRect();
}

int32_t Esp32Graphics::renderText(int32_t x, int32_t baseline, const char* text,
                                  const lgfx::GFXfont& font, uint16_t fg, bool draw) {
    if (draw) {
        lcd_.setFont(&font);
        lcd_.setTextColor(fg);
        lcd_.setTextDatum(lgfx::textdatum_t::baseline_left);
    }
    const int32_t startX = x;
    auto drawAscii = [&](char c) {
        if (draw) {
            const char buffer[2] = {c, '\0'};
            lcd_.drawString(buffer, x, baseline);
        }
        x += glyphFor(font, c).xAdvance;
    };

    while (*text != '\0') {
        const uint16_t codePoint = nextCodePoint(text);
        if (codePoint < 0x80) {
            drawAscii(static_cast<char>(codePoint));
            continue;
        }
        if (const char base = umlautBase(codePoint)) {
            const lgfx::GFXglyph& glyph = glyphFor(font, base);
            if (draw) {
                const int32_t dot = std::max<int32_t>(2, font.yAdvance / 10);
                const int32_t gap = std::max<int32_t>(1, dot / 2);
                const int32_t dotY = baseline + glyph.yOffset - gap - dot;
                const int32_t center = x + glyph.xOffset + glyph.width / 2;
                const int32_t half = std::max<int32_t>(dot, glyph.width / 4);
                lcd_.fillRect(center - half - dot / 2, dotY, dot, dot, fg);
                lcd_.fillRect(center + half - dot / 2, dotY, dot, dot, fg);
            }
            drawAscii(base);
            continue;
        }
        if (codePoint == kSharpS) {
            drawAscii('s');
            drawAscii('s');
            continue;
        }
        if (codePoint == kDegreeSign) {
            const int32_t cap = capHeight(font);
            const int32_t radius = std::max<int32_t>(2, cap / 5);
            if (draw) {
                const int32_t cx = x + radius + 1;
                const int32_t cy = baseline - cap + radius;
                lcd_.drawCircle(cx, cy, radius, fg);
                if (radius >= 3) {
                    lcd_.drawCircle(cx, cy, radius - 1, fg);
                }
            }
            x += 2 * radius + 3;
            continue;
        }
        drawAscii('?');
    }
    return x - startX;
}
