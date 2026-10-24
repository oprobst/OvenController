#pragma once

#include <cstdint>

/// Operator events of the rotary encoder.
enum class InputEvent : uint8_t { None, RotateCw, RotateCcw, Pressed };
