#include "button_debouncer.h"

ButtonDebouncer::ButtonDebouncer(uint32_t debounceMs) : debounceMs_(debounceMs) {}

bool ButtonDebouncer::update(bool levelPressed, Milliseconds now) {
    if (!initialized_) {
        initialized_ = true;
        rawLevel_ = levelPressed;
        stableLevel_ = levelPressed;
        lastChange_ = now;
        return false;
    }
    if (levelPressed != rawLevel_) {
        rawLevel_ = levelPressed;
        lastChange_ = now;
        return false;
    }
    if (stableLevel_ != rawLevel_ && now.elapsedSince(lastChange_) >= debounceMs_) {
        stableLevel_ = rawLevel_;
        return stableLevel_;
    }
    return false;
}
