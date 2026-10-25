#pragma once

// Hardware configuration for PCB revision A: ESP32-S3-WROOM-1U-N8 on the controller board.
// Pin assignment and wiring: specs/002-controller-pcb/contracts/connectors.md;
// rationale: specs/002-controller-pcb/research.md (R3-R15).
// Pins verified against datasheet: no

#include <cstdint>

namespace board {

constexpr char kPcbRevision[] = "A";

// Revision strap: 10k/10k divider at ADC1 (GPIO 1), see research.md R12.
constexpr int8_t kPinRevisionStrap = 1;
constexpr uint16_t kStrapMinMillivolts = 1450;
constexpr uint16_t kStrapMaxMillivolts = 1850;

// Diagnostic LED, active HIGH.
constexpr int8_t kPinDiagLed = 2;

// Heater enable chain (research.md R6-R8). All outputs have external pull-downs.
constexpr int8_t kPinHeatTop = 4;
constexpr int8_t kPinHeatBottom = 5;
constexpr int8_t kPinArm = 6;           // rising edge sets the enable flip-flop
constexpr int8_t kPinDisarm = 18;       // LOW pulls /MR of the TPS3828, clears the flip-flop
constexpr int8_t kPinWatchdogWdi = 21;  // external watchdog input, toggle to feed
constexpr int8_t kPinClearSense = 36;   // LOW while any clear source is active
constexpr int8_t kPinStopNo = 42;       // stop button normally-open contact, LOW = pressed
constexpr int8_t kPinLatchQ = 47;       // HIGH = heating enabled

// TFT ST7789 240x320 on SPI2 via IO-MUX (write only).
constexpr int8_t kPinTftBacklight = 7;  // P-MOSFET gate: LOW = on
constexpr int8_t kPinTftRst = 8;
constexpr int8_t kPinTftDc = 9;
constexpr int8_t kPinTftCs = 10;
constexpr int8_t kPinSpiMosi = 11;
constexpr int8_t kPinSpiSck = 12;
constexpr uint32_t kTftSpiHz = 26666666;  // 80 MHz / 3, cable up to 20 cm
constexpr uint16_t kTftWidthPx = 240;
constexpr uint16_t kTftHeightPx = 320;

// Two MAX31856 on SPI3 via the GPIO matrix, SPI mode 1.
constexpr int8_t kPinThermocoupleMiso = 13;
constexpr int8_t kPinThermocoupleSck = 14;
constexpr int8_t kPinThermocoupleMosi = 15;
constexpr int8_t kPinThermocoupleCsTop = 16;
constexpr int8_t kPinThermocoupleCsBottom = 17;
constexpr uint32_t kThermocoupleSpiHz = 1000000;

// Passive buzzer via N-MOSFET, PWM.
constexpr int8_t kPinBuzzer = 35;

// Rotary encoder (Alps EC11E15244G1) with push button; external pull-ups and RC filters.
constexpr int8_t kPinEncoderA = 39;
constexpr int8_t kPinEncoderB = 40;
constexpr int8_t kPinEncoderButton = 41;
constexpr int32_t kEncoderStepsPerDetent = 2;  // 15 pulses / 30 detents: half a cycle per detent

}  // namespace board
