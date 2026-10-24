#include "esp32_encoder_pins.h"

#include <Arduino.h>
#include <soc/gpio_reg.h>

#include "board/board.h"
#include "quadrature_decoder.h"

namespace {

static_assert(board::kPinEncoderA < 32 && board::kPinEncoderB < 32,
              "encoder ISR reads GPIO_IN_REG, which covers GPIO 0..31 only");

uint8_t IRAM_ATTR readAB() {
    const uint32_t levels = REG_READ(GPIO_IN_REG);
    const uint8_t a = (levels >> board::kPinEncoderA) & 0x1u;
    const uint8_t b = (levels >> board::kPinEncoderB) & 0x1u;
    return static_cast<uint8_t>((a << 1) | b);
}

}  // namespace

Esp32EncoderPins::Esp32EncoderPins() {
    for (int i = 0; i < 16; ++i) {
        transitionTable_[i] = QuadratureDecoder::kTransitionTable[i];
    }
}

void Esp32EncoderPins::begin() {
    pinMode(board::kPinEncoderA, INPUT_PULLUP);
    pinMode(board::kPinEncoderB, INPUT_PULLUP);
    pinMode(board::kPinEncoderButton, INPUT_PULLUP);
    previousAB_ = readAB();
    attachInterruptArg(board::kPinEncoderA, &Esp32EncoderPins::onEdge, this, CHANGE);
    attachInterruptArg(board::kPinEncoderB, &Esp32EncoderPins::onEdge, this, CHANGE);
}

void IRAM_ATTR Esp32EncoderPins::onEdge(void* self) {
    auto* pins = static_cast<Esp32EncoderPins*>(self);
    const uint8_t current = readAB();
    const int8_t step = pins->transitionTable_[(pins->previousAB_ << 2) | current];
    pins->previousAB_ = current;
    if (step != 0) {
        pins->steps_.fetch_add(step, std::memory_order_relaxed);
    }
}

int32_t Esp32EncoderPins::takeDetents() {
    const int32_t steps = steps_.load(std::memory_order_relaxed);
    const int32_t detents = steps / board::kEncoderStepsPerDetent;
    if (detents != 0) {
        steps_.fetch_sub(detents * board::kEncoderStepsPerDetent, std::memory_order_relaxed);
    }
    return detents;
}

bool Esp32EncoderPins::buttonLevelPressed() const {
    return digitalRead(board::kPinEncoderButton) == LOW;
}
