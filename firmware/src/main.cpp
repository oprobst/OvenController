// Composition root: the only place where concrete implementations are created and wired.

#include <Arduino.h>

#include <cstdio>

#include "app_controller.h"
#include "app_version.h"
#include "board/board.h"
#include "esp32_encoder_pins.h"
#include "esp32_graphics.h"
#include "esp32_max31855_bus.h"
#include "esp32_platform.h"
#include "esp32_sntp_clock.h"
#include "esp32_wifi_station.h"
#include "logger.h"
#include "main_menu.h"
#include "null_logger.h"
#include "oven_temperature_monitor.h"
#include "rotary_input.h"

namespace {

Esp32Clock systemClock;
Esp32SerialOut serialOut;
Esp32SystemInfo systemInfo;
Esp32Watchdog watchdog;
Esp32Graphics graphics;
#if OVEN_LOG_LEVEL > 0
Logger logger(serialOut, systemClock);
#else
NullLogger logger;
#endif

Esp32Max31855Bus thermocoupleBus;
OvenTemperatureMonitor ovenTemperature(thermocoupleBus, logger);
Esp32WifiStation wifi(logger);
Esp32SntpClock wallClock(logger);
Esp32EncoderPins encoderPins;
RotaryInput rotaryInput(encoderPins);
MainMenu mainMenu;
const FirmwareIdentity identity{kFirmwareVersion, board::kPcbRevision};

AppController app(watchdog, ovenTemperature, wifi, wallClock, rotaryInput, graphics, logger,
                  mainMenu, identity);

}  // namespace

void setup() {
    serialOut.begin();
    char fields[64];
    std::snprintf(fields, sizeof(fields), "fw=%s pcb=%s reset=%s", kFirmwareVersion,
                  board::kPcbRevision, systemInfo.resetReason());
    logger.log(LogLevel::Info, "sys", "boot", fields);

    watchdog.start();
    graphics.begin();
    thermocoupleBus.begin();
    encoderPins.begin();

    // Main menu entries of feature 001; later features add their entries here (FR-012).
    mainMenu.add({MenuEntryId::Heat, "Heizen"});
    mainMenu.add({MenuEntryId::Profile, "Profil"});
    mainMenu.add({MenuEntryId::Setup, "Setup"});

    app.setup(systemClock.now());
}

void loop() { app.tick(systemClock.now()); }
