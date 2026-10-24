#include "ui_state_machine.h"

#include <cstdio>

namespace {

struct Transition {
    UiScreen from;
    UiEvent event;
    UiScreen to;
};

// The only source of screen transitions. Must match docs/state-machine.md.
constexpr Transition kTransitions[] = {
    {UiScreen::Boot, UiEvent::DisplayReady, UiScreen::Welcome},
    {UiScreen::Welcome, UiEvent::WelcomeTimeout, UiScreen::MainMenu},
    {UiScreen::Welcome, UiEvent::ButtonPressed, UiScreen::MainMenu},
    {UiScreen::MainMenu, UiEvent::RotateCw, UiScreen::MainMenu},
    {UiScreen::MainMenu, UiEvent::RotateCcw, UiScreen::MainMenu},
    {UiScreen::MainMenu, UiEvent::ButtonPressed, UiScreen::MainMenu},
};

}  // namespace

UiStateMachine::UiStateMachine(ILogger& logger) : logger_(logger) {}

bool UiStateMachine::handle(UiEvent event) {
    for (const Transition& transition : kTransitions) {
        if (transition.from == current_ && transition.event == event) {
            current_ = transition.to;
            return true;
        }
    }
    char fields[64];
    std::snprintf(fields, sizeof(fields), "screen=%s event=%s", screenName(current_),
                  eventName(event));
    logger_.log(LogLevel::Debug, "ui", "transition rejected", fields);
    return false;
}

size_t UiStateMachine::transitionCount() { return sizeof(kTransitions) / sizeof(kTransitions[0]); }

const char* UiStateMachine::screenName(UiScreen screen) {
    switch (screen) {
        case UiScreen::Boot:
            return "boot";
        case UiScreen::Welcome:
            return "welcome";
        case UiScreen::MainMenu:
            return "main_menu";
    }
    return "unknown";
}

const char* UiStateMachine::eventName(UiEvent event) {
    switch (event) {
        case UiEvent::DisplayReady:
            return "display_ready";
        case UiEvent::WelcomeTimeout:
            return "welcome_timeout";
        case UiEvent::ButtonPressed:
            return "button_pressed";
        case UiEvent::RotateCw:
            return "rotate_cw";
        case UiEvent::RotateCcw:
            return "rotate_ccw";
    }
    return "unknown";
}
