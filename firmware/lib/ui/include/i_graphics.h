#pragma once

#include <cstdint>

/// Logical color roles; the display implementation maps them to concrete colors.
enum class Color : uint8_t { Background, Foreground, Accent, Warning, Error, Muted };

/// Logical font sizes.
enum class FontSize : uint8_t { Small, Medium, Large };

/// Horizontal text alignment inside a box.
enum class Align : uint8_t { Left, Center, Right };

/// Minimal drawing interface for the user interface (implemented in the HAL).
/// Coordinates are in pixels, origin top left; all calls are synchronous and non-allocating.
class IGraphics {
public:
    virtual ~IGraphics() = default;

    /// @return display width in pixels.
    virtual uint16_t width() const = 0;

    /// @return display height in pixels.
    virtual uint16_t height() const = 0;

    /// Fills a rectangle.
    virtual void fillRect(int16_t x, int16_t y, uint16_t w, uint16_t h, Color color) = 0;

    /// Fills the box with bg and draws text (UTF-8, German umlauts and the degree sign) inside it,
    /// vertically centered and horizontally aligned. Text outside the box is clipped.
    virtual void drawText(int16_t x, int16_t y, uint16_t w, uint16_t h, const char* text,
                          FontSize size, Color fg, Color bg, Align align) = 0;
};
