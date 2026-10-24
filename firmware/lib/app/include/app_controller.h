#pragma once

#include "firmware_identity.h"
#include "i_graphics.h"
#include "i_logger.h"
#include "i_network_status.h"
#include "i_rotary_input.h"
#include "i_temperature_sensor.h"
#include "i_wall_clock.h"
#include "i_watchdog.h"
#include "main_menu.h"
#include "menu_screen.h"
#include "scheduler.h"
#include "status_bar.h"
#include "system_status.h"
#include "ui_state_machine.h"
#include "welcome_screen.h"

/// Application logic of feature 001: welcome screen with status area, then main menu.
/// Never switches any heater (FR-014). All work happens in non-blocking periodic tasks.
class AppController {
public:
    /// All references must outlive the controller.
    AppController(IWatchdog& watchdog, ITemperatureSensor& temperature, INetworkStatus& network,
                  IWallClock& wallClock, IRotaryInput& input, IGraphics& graphics, ILogger& logger,
                  MainMenu& menu, const FirmwareIdentity& identity);

    /// Draws the welcome screen, then starts WiFi and time synchronization and registers the
    /// periodic tasks. The display must already be initialized. Call once.
    void setup(Milliseconds now);

    /// Runs all due tasks. Call from the main loop as often as possible.
    void tick(Milliseconds now);

    /// @return current screen.
    UiScreen screen() const { return stateMachine_.current(); }

private:
    static void feedWatchdogTask(void* self, Milliseconds now);
    static void inputTask(void* self, Milliseconds now);
    static void temperatureTask(void* self, Milliseconds now);
    static void networkTask(void* self, Milliseconds now);
    static void displayTask(void* self, Milliseconds now);

    void handleInput(Milliseconds now);
    void dispatch(UiEvent event);
    void refreshStatus();
    void clearContent();

    IWatchdog& watchdog_;
    ITemperatureSensor& temperature_;
    INetworkStatus& network_;
    IWallClock& wallClock_;
    IRotaryInput& input_;
    IGraphics& graphics_;
    ILogger& logger_;
    MainMenu& menu_;
    const FirmwareIdentity& identity_;

    Scheduler scheduler_;
    UiStateMachine stateMachine_;
    StatusBar statusBar_;
    WelcomeScreen welcomeScreen_;
    MenuScreen menuScreen_;
    SystemStatus status_{};
    Milliseconds welcomeShownAt_{0};
};
