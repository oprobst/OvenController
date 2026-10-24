#pragma once

// Hardware configuration for PCB revision DEV: breadboard prototype on an ESP32-DevKitC V4.
// Pin assignment rationale: specs/001-startup-status-menu/research.md (R13).
// Pins verified against datasheet: no

#include <cstdint>

namespace board {

constexpr char kPcbRevision[] = "DEV";

// TFT ST7789 240x320 on VSPI (write only, MISO not used).
constexpr int8_t kPinSpiSck = 18;
constexpr int8_t kPinSpiMosi = 23;
constexpr int8_t kPinTftCs = 22;
constexpr int8_t kPinTftDc = 21;
constexpr int8_t kPinTftRst = 4;
constexpr int8_t kPinTftBacklight = 32;
constexpr uint32_t kTftSpiHz = 40000000;
constexpr uint16_t kTftWidthPx = 240;
constexpr uint16_t kTftHeightPx = 320;

// MAX31855 on its own bit-banged lines, independent of the display bus.
constexpr int8_t kPinMax31855Sck = 14;
constexpr int8_t kPinMax31855So = 34;  // input-only pin, driven by the MAX31855
constexpr int8_t kPinMax31855Cs = 33;

// Rotary encoder with push button, internal pull-ups, active LOW.
constexpr int8_t kPinEncoderA = 25;
constexpr int8_t kPinEncoderB = 26;
constexpr int8_t kPinEncoderButton = 27;
constexpr int32_t kEncoderStepsPerDetent = 4;  // KY-040 style: one full quadrature cycle per detent

}  // namespace board
