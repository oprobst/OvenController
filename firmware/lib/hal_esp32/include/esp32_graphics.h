#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#include "i_graphics.h"

/// IGraphics on the 2.8" ST7789 TFT (240 x 320, portrait) using LovyanGFX without sprites.
/// The DejaVu fonts of LovyanGFX only contain ASCII; German umlauts, sharp s and the degree
/// sign are composed from ASCII glyphs and small drawn shapes.
class Esp32Graphics final : public IGraphics {
public:
    Esp32Graphics();

    /// Initializes panel, bus and backlight and clears the screen. Call once during setup.
    void begin();

    uint16_t width() const override;
    uint16_t height() const override;
    void fillRect(int16_t x, int16_t y, uint16_t w, uint16_t h, Color color) override;
    void drawText(int16_t x, int16_t y, uint16_t w, uint16_t h, const char* text, FontSize size,
                  Color fg, Color bg, Align align) override;

private:
    class Device : public lgfx::LGFX_Device {
    public:
        Device();

    private:
        lgfx::Panel_ST7789 panel_;
        lgfx::Bus_SPI bus_;
        lgfx::Light_PWM light_;
    };

    int32_t renderText(int32_t x, int32_t baseline, const char* text, const lgfx::GFXfont& font,
                       uint16_t fg, bool draw);

    Device lcd_;
};
