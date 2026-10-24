#include "esp32_max31855_bus.h"

#include <Arduino.h>

#include "board/board.h"

void Esp32Max31855Bus::begin() {
    pinMode(board::kPinMax31855Cs, OUTPUT);
    digitalWrite(board::kPinMax31855Cs, HIGH);
    pinMode(board::kPinMax31855Sck, OUTPUT);
    digitalWrite(board::kPinMax31855Sck, LOW);
    pinMode(board::kPinMax31855So, INPUT);
}

uint32_t Esp32Max31855Bus::readRaw() {
    // MAX31855: D31 is valid after CS falls; each falling SCK edge shifts out the next bit.
    // digitalWrite takes well over the minimum clock high/low time of 100 ns.
    uint32_t raw = 0;
    digitalWrite(board::kPinMax31855Cs, LOW);
    for (int bit = 0; bit < 32; ++bit) {
        raw = (raw << 1) | (digitalRead(board::kPinMax31855So) == HIGH ? 1u : 0u);
        digitalWrite(board::kPinMax31855Sck, HIGH);
        digitalWrite(board::kPinMax31855Sck, LOW);
    }
    digitalWrite(board::kPinMax31855Cs, HIGH);
    return raw;
}
