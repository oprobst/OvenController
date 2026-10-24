#pragma once

#include "app_controller.h"
#include "fake_graphics.h"
#include "fake_input.h"
#include "fake_logger.h"
#include "fake_network.h"
#include "fake_temperature_sensor.h"
#include "fake_watchdog.h"

/// AppController wired with fakes and the three feature-001 menu entries.
struct AppFixture {
    AppFixture() {
        menu.add({MenuEntryId::Heat, "Heizen"});
        menu.add({MenuEntryId::Profile, "Profil"});
        menu.add({MenuEntryId::Setup, "Setup"});
    }

    /// Calls tick() every 5 ms from the current time until untilMs (inclusive).
    void runUntil(uint32_t untilMs) {
        for (; nowMs <= untilMs; nowMs += 5) {
            app.tick(Milliseconds{nowMs});
        }
    }

    FakeWatchdog watchdog;
    FakeTemperatureSensor temperature;
    FakeNetworkStatus network;
    FakeWallClock wallClock;
    FakeRotaryInput input;
    FakeGraphics graphics;
    FakeLogger logger;
    MainMenu menu;
    FirmwareIdentity identity{"0.1.0", "DEV"};
    AppController app{watchdog, temperature, network, wallClock, input,
                      graphics, logger,      menu,    identity};
    uint32_t nowMs = 0;
};
