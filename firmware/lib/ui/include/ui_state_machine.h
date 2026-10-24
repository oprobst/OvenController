#pragma once

#include <cstddef>
#include <cstdint>

#include "i_logger.h"

/// Screens of the user interface (constitution principle VIII; docs/state-machine.md).
enum class UiScreen : uint8_t { Boot, Welcome, MainMenu };

/// Events driving the screen state machine.
enum class UiEvent : uint8_t { DisplayReady, WelcomeTimeout, ButtonPressed, RotateCw, RotateCcw };

/// Explicit state machine for the screen flow with one central transition table.
/// Undefined transitions are rejected, leave the state unchanged and are logged on DEBUG.
class UiStateMachine {
public:
    /// @param logger diagnostics; must outlive the state machine
    explicit UiStateMachine(ILogger& logger);

    /// Applies an event.
    /// @return true if the transition is defined (state may stay the same), false if rejected
    bool handle(UiEvent event);

    /// @return current screen.
    UiScreen current() const { return current_; }

    /// @return number of rows in the transition table.
    static size_t transitionCount();

    /// @return log name of a screen, e.g. "welcome".
    static const char* screenName(UiScreen screen);

    /// @return log name of an event, e.g. "rotate_cw".
    static const char* eventName(UiEvent event);

private:
    ILogger& logger_;
    UiScreen current_ = UiScreen::Boot;
};
