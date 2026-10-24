#include "app_controller.h"

#include <cstdio>

#include "app_config.h"
#include "screen_layout.h"
#include "status_formatter.h"

AppController::AppController(IWatchdog& watchdog, ITemperatureSensor& temperature,
                             INetworkStatus& network, IWallClock& wallClock, IRotaryInput& input,
                             IGraphics& graphics, ILogger& logger, MainMenu& menu,
                             const FirmwareIdentity& identity)
    : watchdog_(watchdog),
      temperature_(temperature),
      network_(network),
      wallClock_(wallClock),
      input_(input),
      graphics_(graphics),
      logger_(logger),
      menu_(menu),
      identity_(identity),
      stateMachine_(logger),
      statusBar_(graphics),
      welcomeScreen_(graphics),
      menuScreen_(graphics) {}

void AppController::setup(Milliseconds now) {
    // Welcome first, so it appears before anything that may take time (FR-008, SC-001).
    graphics_.fillRect(0, 0, layout::kScreenWidth, layout::kScreenHeight, Color::Background);
    stateMachine_.handle(UiEvent::DisplayReady);
    refreshStatus();
    statusBar_.draw(StatusFormatter::format(status_));
    welcomeScreen_.draw(identity_);
    welcomeShownAt_ = now;

    network_.begin();
    wallClock_.begin();

    scheduler_.add(&AppController::feedWatchdogTask, this, 0);
    scheduler_.add(&AppController::inputTask, this, config::kInputPollPeriodMs);
    scheduler_.add(&AppController::temperatureTask, this, config::kTemperatureSamplePeriodMs);
    scheduler_.add(&AppController::networkTask, this, config::kNetworkPollPeriodMs);
    scheduler_.add(&AppController::displayTask, this, config::kDisplayRefreshPeriodMs);
}

void AppController::tick(Milliseconds now) { scheduler_.tick(now); }

void AppController::feedWatchdogTask(void* self, Milliseconds /*now*/) {
    static_cast<AppController*>(self)->watchdog_.feed();
}

void AppController::inputTask(void* self, Milliseconds now) {
    static_cast<AppController*>(self)->handleInput(now);
}

void AppController::temperatureTask(void* self, Milliseconds now) {
    static_cast<AppController*>(self)->temperature_.sample(now);
}

void AppController::networkTask(void* self, Milliseconds /*now*/) {
    auto* app = static_cast<AppController*>(self);
    app->network_.poll();
    app->wallClock_.poll();
}

void AppController::displayTask(void* self, Milliseconds /*now*/) {
    auto* app = static_cast<AppController*>(self);
    app->refreshStatus();
    app->statusBar_.draw(StatusFormatter::format(app->status_));
}

void AppController::handleInput(Milliseconds now) {
    input_.poll(now);
    for (InputEvent event = input_.nextEvent(); event != InputEvent::None;
         event = input_.nextEvent()) {
        switch (event) {
            case InputEvent::RotateCw:
                dispatch(UiEvent::RotateCw);
                break;
            case InputEvent::RotateCcw:
                dispatch(UiEvent::RotateCcw);
                break;
            case InputEvent::Pressed:
                dispatch(UiEvent::ButtonPressed);
                break;
            case InputEvent::None:
                break;
        }
    }
    if (stateMachine_.current() == UiScreen::Welcome &&
        now.elapsedSince(welcomeShownAt_) >= config::kWelcomeDurationMs) {
        dispatch(UiEvent::WelcomeTimeout);
    }
}

void AppController::dispatch(UiEvent event) {
    const UiScreen before = stateMachine_.current();
    if (!stateMachine_.handle(event)) {
        return;
    }
    const UiScreen after = stateMachine_.current();

    if (before == UiScreen::Welcome && after == UiScreen::MainMenu) {
        clearContent();
        menuScreen_.draw(menu_);
        return;
    }
    if (after != UiScreen::MainMenu) {
        return;
    }
    if (event == UiEvent::RotateCw || event == UiEvent::RotateCcw) {
        menu_.moveBy(event == UiEvent::RotateCw ? 1 : -1);
        menuScreen_.update(menu_);
    } else if (event == UiEvent::ButtonPressed && menu_.count() > 0) {
        // Feature 001: all entries are placeholders without any action.
        char fields[48];
        std::snprintf(fields, sizeof(fields), "entry=%s action=none",
                      MainMenu::logName(menu_.select()));
        logger_.log(LogLevel::Info, "ui", "menu select", fields);
    }
}

void AppController::refreshStatus() {
    status_.wifi = network_.state();
    status_.clock = wallClock_.now();
    status_.oven = temperature_.current();
}

void AppController::clearContent() {
    graphics_.fillRect(layout::kContent.x, layout::kContent.y, layout::kContent.w,
                       layout::kContent.h, Color::Background);
    statusBar_.invalidate();
    statusBar_.draw(StatusFormatter::format(status_));
}
