#include "rotary_input.h"

#include "app_config.h"

RotaryInput::RotaryInput(IEncoderPins& pins) : pins_(pins), debouncer_(config::kButtonDebounceMs) {}

void RotaryInput::poll(Milliseconds now) {
    int32_t detents = pins_.takeDetents();
    for (; detents > 0; --detents) {
        push(InputEvent::RotateCw);
    }
    for (; detents < 0; ++detents) {
        push(InputEvent::RotateCcw);
    }
    if (debouncer_.update(pins_.buttonLevelPressed(), now)) {
        push(InputEvent::Pressed);
    }
}

InputEvent RotaryInput::nextEvent() {
    if (size_ == 0) {
        return InputEvent::None;
    }
    const InputEvent event = queue_[head_];
    head_ = (head_ + 1) % kQueueSize;
    --size_;
    return event;
}

void RotaryInput::push(InputEvent event) {
    if (size_ >= kQueueSize) {
        return;
    }
    queue_[(head_ + size_) % kQueueSize] = event;
    ++size_;
}
