---

description: "Task list for feature 001-startup-status-menu"
---

# Tasks: Startbildschirm mit Statusanzeige und Hauptmenü

**Input**: Design documents from `specs/001-startup-status-menu/`

**Prerequisites**: [plan.md](plan.md), [spec.md](spec.md), [research.md](research.md),
[data-model.md](data-model.md), [contracts/](contracts/), [quickstart.md](quickstart.md)

**Tests**: Enthalten. Die Constitution verlangt Unit-Tests je Modul gegen seine Schnittstelle
(Prinzip III), testgetriebene Entwicklung für sicherheitsrelevante Logik (Fühler-
Fehlererkennung) und Host-Tests für jeden Übergang der Zustandsmaschine (Prinzip VIII).
Test-Tasks stehen jeweils **vor** der Implementierung und müssen zuerst fehlschlagen.

**Organization**: Tasks sind nach User Story gruppiert.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: parallel ausführbar (andere Dateien, keine offenen Abhängigkeiten)
- **[Story]**: US1 = Begrüßung und Statusübersicht, US2 = Übergang ins Hauptmenü

## Path Conventions

- Firmware-Projekt unter `firmware/`; Module als PlatformIO-Bibliotheken
  `firmware/lib/<modul>/include/` (öffentlich) und `firmware/lib/<modul>/src/`
- Host-Tests unter `firmware/test/test_<modul>/`, Fakes unter `firmware/test/fakes/`,
  HIL-Tests unter `firmware/test/hil/`
- Code-Bezeichner, Kommentare und Log-Texte englisch; UI-Texte deutsch; öffentliche
  Interfaces mit Doxygen (Zweck, Parameter mit Einheiten, Rückgabe, Fehlerverhalten)

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Firmware-Projekt, Werkzeuge und zentrale Konfiguration anlegen

- [X] T001 Verzeichnisstruktur laut [plan.md](plan.md) anlegen: `firmware/include/board/`, `firmware/src/`, `firmware/lib/{units,diagnostics,platform,temperature,network,input,ui,app,hal_esp32}/{include,src}/`, `firmware/test/fakes/`, `firmware/test/hil/` (leere Verzeichnisse mit `.gitkeep`, wo nötig)
- [X] T002 `firmware/platformio.ini` anlegen: `[platformio] default_envs = esp32_revdev`; gemeinsame `[env]`-Sektion mit `build_flags = -std=gnu++17 -fno-exceptions -fno-rtti -Wall -Wextra -DOVEN_LOG_LEVEL=3`, `build_unflags = -std=gnu++11 -fexceptions -frtti`, `test_framework = unity`, `check_tool = cppcheck`, `check_flags = cppcheck: --enable=warning,style,performance`, `check_severity = medium, high`; `[env:esp32_revdev]` mit `platform = espressif32@6.9.0`, `board = esp32dev`, `framework = arduino`, `monitor_speed = 115200`, `lib_deps = lovyan03/LovyanGFX@1.1.16`, `build_flags = ${env.build_flags} -DOVEN_BOARD_REV_DEV`, `test_filter = hil/*`; `[env:native]` mit `platform = native`, `lib_ignore = hal_esp32`, `test_ignore = hil/*`, `build_src_filter = -<*>` (kein `main.cpp` im Host-Build)
- [X] T003 [P] `.clang-format` im Repository-Root anlegen (`BasedOnStyle: Google`, `IndentWidth: 4`, `ColumnLimit: 100`, `AccessModifierOffset: -4`, `DerivePointerAlignment: false`, `PointerAlignment: Left`)
- [X] T004 [P] `.gitignore` ergänzen um `firmware/include/wifi_secrets.h`
- [X] T005 [P] `firmware/include/wifi_secrets.example.h` anlegen: `#pragma once`, Kommentar „Copy to wifi_secrets.h and fill in; never commit wifi_secrets.h", `namespace secrets { constexpr char kWifiSsid[] = ""; constexpr char kWifiPassword[] = ""; }`
- [X] T006 [P] `firmware/include/app_version.h` anlegen: `constexpr char kFirmwareVersion[] = "0.1.0";` (Semantic Versioning, Prinzip VI)
- [X] T007 [P] `firmware/include/board/board_rev_dev.h` anlegen mit allen `constexpr`-Werten aus [research.md R13](research.md#r13--pinbelegung-rev-dev-esp32-devkitc-v4): `kPcbRevision[] = "DEV"`, `kPinSpiSck = 18`, `kPinSpiMosi = 23`, `kPinSpiMiso = 19`, `kPinTftCs = 22`, `kPinTftDc = 21`, `kPinTftRst = 4`, `kPinTftBacklight = 32`, `kPinMax31855Cs = 33`, `kPinEncoderA = 25`, `kPinEncoderB = 26`, `kPinEncoderButton = 27`, `kTftSpiHz = 40000000`, `kMax31855SpiHz = 4000000`, `kTftWidthPx = 240`, `kTftHeightPx = 320` (Hochformat); Kommentar „pins verified against datasheet: no" bis zur Prüfung; zusätzlich `firmware/include/board/board.h`, das per `#if defined(OVEN_BOARD_REV_DEV)` den passenden Header einbindet und sonst `#error "unsupported PCB revision"` auslöst
- [X] T008 [P] `firmware/include/app_config.h` anlegen mit allen zentralen Konstanten (Prinzip VII) als `constexpr` mit Einheit im Namen: `kWelcomeDurationMs = 3000`, `kWatchdogTimeoutMs = 5000`, `kTemperatureSamplePeriodMs = 250`, `kNetworkPollPeriodMs = 1000`, `kDisplayRefreshPeriodMs = 100`, `kInputPollPeriodMs = 5`, `kButtonDebounceMs = 20`, `kPlausibleMinCelsius = -20.0f`, `kPlausibleMaxCelsius = 350.0f`, `kFaultDebounceSamples = 2`, `kRecoverSamples = 2`, `kStaleTimeoutMs = 1000`, `kMinValidYear = 2026`, `kTimeZonePosix[] = "CET-1CEST,M3.5.0,M10.5.0/3"`, `kNtpServer1[] = "de.pool.ntp.org"`, `kNtpServer2[] = "pool.ntp.org"`, `kMaxMenuEntries = 8`, `kLogLineMaxChars = 255`, `kSerialBaud = 115200`, `kSerialTxBufferBytes = 2048`
- [X] T009 [P] CI-Workflow `.github/workflows/firmware.yml` anlegen (Trigger: push, pull_request; ubuntu-latest; Python + `pip install platformio==6.1.19`; Cache `~/.platformio`): Schritte `pio run -d firmware -e esp32_revdev`, `pio test -d firmware -e native`, `pio check -d firmware -e esp32_revdev --fail-on-defect=medium`, `clang-format --dry-run --Werror` über `firmware/**/*.{h,cpp}`; der Build erzeugt vorher `firmware/include/wifi_secrets.h` **nicht** (CI testet den Fall „nicht eingerichtet")

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Grundtypen, Diagnose, Plattform-Interfaces, Scheduler, Grafik-HAL und Composition Root, auf denen beide User Stories aufbauen

**⚠️ CRITICAL**: Keine User-Story-Arbeit vor Abschluss dieser Phase

### Tests (zuerst schreiben, müssen fehlschlagen)

- [X] T010 [P] Host-Tests für `Celsius` und `Milliseconds` in `firmware/test/test_units/test_units.cpp`: Rundung auf ganze Grad kaufmännisch (23,49 → 23; 23,5 → 24; −0,5 → −1), überlaufsichere Differenz `elapsed(now, since)` über den 32-Bit-Überlauf hinweg
- [X] T011 [P] Host-Tests für `LogFormatter` in `firmware/test/test_diagnostics/test_log_formatter.cpp` gegen [contracts/log-format.md](contracts/log-format.md): Format `t=<ms> lvl=<LEVEL> mod=<modul> msg="<text>"[ <key>=<value>]*` mit `\n`; Maskierung von `"` und `\` in `msg`; Zeilen über 255 Zeichen werden abgeschnitten und mit `trunc=1` markiert; Level-Namen `ERROR`, `WARN`, `INFO`, `DEBUG`
- [X] T012 [P] Host-Tests für `Logger` in `firmware/test/test_diagnostics/test_logger.cpp` mit `FakeSerialOut`: Level-Filter nach Compile-Zeit-Level; verworfene Zeilen bei vollem Puffer erhöhen den Zähler und erzeugen beim nächsten erfolgreichen Schreiben `mod=log msg="dropped" count=<n>`; `log()` blockiert nie
- [X] T013 [P] Host-Tests für `Scheduler` in `firmware/test/test_app/test_scheduler.cpp`: Aufgaben laufen mit ihrer Periode, auch über den `millis()`-Überlauf; maximal 8 Aufgaben (9. wird abgelehnt mit Rückgabewert `false`); keine Nachholläufe nach langer Pause (höchstens ein Lauf pro `tick`)

### Implementierung

- [X] T014 [P] `firmware/lib/units/include/units.h` (header-only): `struct Celsius { float value; }` mit `int32_t roundedWholeDegrees() const`; `struct Milliseconds { uint32_t value; }` mit `uint32_t elapsedSince(Milliseconds earlier) const` (Differenz modulo 2³²)
- [X] T015 [P] `firmware/lib/platform/include/` anlegen: `i_clock.h`, `i_watchdog.h`, `i_serial_out.h`, `i_system_info.h` exakt laut [contracts/interfaces.md § platform](contracts/interfaces.md#platform) mit Doxygen
- [X] T016 `firmware/lib/diagnostics/include/error_code.h`: `enum class ErrorCode : uint16_t { None = 0, ThermocoupleOpen = 101, ThermocoupleShortGnd = 102, ThermocoupleShortVcc = 103, TemperatureImplausible = 104, SensorNoResponse = 105 };` plus `void formatErrorCode(ErrorCode, char* out, size_t len)` → `"E101"` usw. (Werte stabil, nie umdeuten)
- [X] T017 `firmware/lib/diagnostics/include/i_logger.h` (`LogLevel`, `ILogger` laut Contract), `firmware/lib/diagnostics/include/log_formatter.h` + `src/log_formatter.cpp` (formatiert in festen Puffer von 256 Byte), `include/logger.h` + `src/logger.cpp` (`Logger(ISerialOut&, IClock&)`, Drop-Zähler), `include/null_logger.h` (leere Implementierung, aktiv bei `OVEN_LOG_LEVEL=0`) – macht T011/T012 grün
- [X] T018 [P] Fakes in `firmware/test/fakes/`: `fake_clock.h` (setzbare Zeit), `fake_serial_out.h` (Puffer mit einstellbarer Kapazität), `fake_logger.h` (zeichnet Aufrufe auf), `fake_watchdog.h` (zählt `feed()`), `fake_graphics.h` (zeichnet `fillRect`/`drawText`-Aufrufe mit Koordinaten, Text, Farben auf)
- [X] T019 `firmware/lib/app/include/scheduler.h` + `src/scheduler.cpp`: festes Array mit 8 Einträgen (Callback als Funktionszeiger + `void* context`, Periode in ms, letzter Lauf); `bool add(...)`, `void tick(Milliseconds now)` – macht T013 grün
- [X] T020 [P] `firmware/lib/ui/include/i_graphics.h` mit `Color`, `FontSize`, `Align`, `IGraphics` laut [contracts/interfaces.md § ui](contracts/interfaces.md#ui)
- [X] T021 [P] HAL Plattform in `firmware/lib/hal_esp32/`: `esp32_clock.{h,cpp}` (`millis()`), `esp32_watchdog.{h,cpp}` (`esp_task_wdt_init(kWatchdogTimeoutMs/1000, true)` + `esp_task_wdt_add(nullptr)`, `feed()` = `esp_task_wdt_reset()`), `esp32_serial_out.{h,cpp}` (`Serial.setTxBufferSize(kSerialTxBufferBytes)` vor `begin(kSerialBaud)`; `tryWrite` schreibt nur, wenn `availableForWrite() >= len`), `esp32_system_info.{h,cpp}` (`esp_reset_reason()` → `"power_on"`, `"brownout"`, `"task_wdt"`, `"int_wdt"`, `"panic"`, `"sw"`, `"ext"`, `"unknown"`)
- [X] T022 HAL Grafik `firmware/lib/hal_esp32/esp32_graphics.{h,cpp}`: LovyanGFX-Panel `Panel_ST7789` + `Bus_SPI` (VSPI, Pins und Takt aus `board.h`, Hochformat `setRotation(0)`, 240×320), Hintergrundbeleuchtung an `kPinTftBacklight`; `Color` → RGB565 (Background `0x0841`, Foreground `0xFFFF`, Accent `0x04DF`, Warning `0xFFE0`, Error `0xF800`, Muted `0x8410`); `drawText` mit UTF-8-fähiger Schrift inkl. Umlauten und `°` (z. B. `lgfx::fonts::efontDE_*` oder DejaVu-Fonts von LovyanGFX), zeichnet Hintergrund der Box mit; keine Sprites
- [X] T023 Composition Root `firmware/src/main.cpp` als Gerüst: statische Instanzen von Clock, SerialOut, Logger (bzw. NullLogger bei `OVEN_LOG_LEVEL=0`), SystemInfo, Watchdog, Graphics; in `setup()` Serial/Logger starten, `mod=sys msg="boot" fw=<version> pcb=<rev> reset=<grund>` loggen, Watchdog starten, Display initialisieren; `loop()` füttert nur den Watchdog (wird in US1 durch `AppController::tick` ersetzt). Keine globalen veränderlichen Variablen außer den Instanzen hier
- [X] T024 [P] `docs/error-codes.md` anlegen: Tabelle E101–E105 mit Bedeutung, Ursache und Abhilfe laut [data-model.md § ErrorCode](data-model.md#errorcode-libdiagnostics); Hinweis, dass Codes nie umgedeutet oder wiederverwendet werden
- [X] T025 [P] HIL-Test Watchdog `firmware/test/hil/test_hil_watchdog/test_main.cpp`: nach Neustart mit Reset-Grund `task_wdt` → Test bestanden; sonst Watchdog starten und Schleife ohne `feed()` blockieren (Neustart innerhalb von 6 s erwartet); Ablauf im Kommentar beschreiben

**Checkpoint**: `pio test -e native` grün für units, diagnostics, scheduler; `pio run -e esp32_revdev` baut; Board bootet, loggt `msg="boot"`, Display wird initialisiert

---

## Phase 3: User Story 1 - Begrüßung und Statusübersicht beim Einschalten (Priority: P1) 🎯 MVP

**Goal**: Nach dem Einschalten Begrüßung mit Firmware-Version und PCB-Revision sowie Statuszeile mit WLAN-Zustand, Uhrzeit/Datum und Ofentemperatur bzw. Fühlerfehler

**Independent Test**: Board einschalten; Begrüßung erscheint ≤ 2 s und bleibt stehen (Menü kommt erst mit US2); Statuszeile zeigt WLAN-Zustand, nach Verbindung Uhrzeit/Datum, Temperatur ≥ 1×/s aktualisiert; abgezogener Fühler → `E101` (Abnahme [quickstart.md](quickstart.md) Schritte 1, 6–13)

### Tests for User Story 1 (zuerst schreiben, müssen fehlschlagen) ⚠️

- [X] T026 [P] [US1] TDD-Tests für `Max31855Decoder` in `firmware/test/test_temperature/test_max31855_decoder.cpp`: Rohwort `0x00000000` und `0xFFFFFFFF` → `SensorNoResponse` (105); Bit 16 gesetzt mit Bit 0 → `ThermocoupleOpen` (101), Bit 1 → `ThermocoupleShortGnd` (102), Bit 2 → `ThermocoupleShortVcc` (103); 14-Bit-Temperatur (Bits 31–18, vorzeichenbehaftet) × 0,25 °C für +25,00 °C, +1023,75 °C, −0,25 °C, −250,00 °C; 12-Bit-Vergleichsstelle (Bits 15–4) × 0,0625 °C für +25,0625 °C und −0,0625 °C
- [X] T027 [P] [US1] TDD-Tests für `OvenTemperatureMonitor` in `firmware/test/test_temperature/test_oven_temperature_monitor.cpp` mit `FakeThermocoupleBus` und `FakeLogger`: vor erster gültiger Messung `valid = false, error = None`; „2 gültige Messungen in Folge → `valid = true`"; „2 fehlerhafte Messungen in Folge → `valid = false`, `error` = Code der letzten Fehlmessung"; eine einzelne Fehlmessung ändert nichts; Werte < −20 °C bzw. > 350 °C → 104; „Seit 1000 ms kein gültiger Wert → `valid = false`, `error = 105`"; Fehler wird nach ≤ 2 Abtastungen à 250 ms gemeldet (≤ 500 ms, SC-005); Fehlerwechsel erzeugen genau eine `WARN`-Zeile `msg="sensor fault" code=E1xx` bzw. eine `INFO`-Zeile `msg="sensor ok"`
- [X] T028 [P] [US1] Host-Tests für `StatusFormatter` in `firmware/test/test_ui/test_status_formatter.cpp` gegen [contracts/ui-screens.md](contracts/ui-screens.md): WLAN-Texte „WLAN verbunden" / „WLAN nicht verbunden" / „WLAN nicht eingerichtet" mit Farben Foreground/Warning/Muted; Uhrzeit `HH:MM` und Datum `TT.MM.JJJJ` als getrennte Texte mit führenden Nullen; `Connected` + Zeit ungültig → `--:--` bzw. `--.--.----` (Muted); nicht verbunden + nie synchronisiert → leer; nicht verbunden + Zeit gültig → laufende Zeit; Temperatur `23 °C` (gerundet), vor erster Messung `--- °C` (Muted), bei Fehler `E101` (Error); nie ein Datum vor 2026 (FR-005)
- [X] T029 [P] [US1] Host-Tests für `StatusBar` und `WelcomeScreen` in `firmware/test/test_ui/test_screens_welcome.cpp` mit `FakeGraphics`: Statusbereich y = 0 … 47 in zwei Zeilen mit Trennlinie bei y = 48 – Zeile 1: WLAN links 160 px, Uhrzeit rechts 80 px; Zeile 2: Datum links 120 px, Temperatur rechts 120 px (siehe [contracts/ui-screens.md](contracts/ui-screens.md)); ein Bereich wird nur neu gezeichnet, wenn sich sein Text geändert hat; Begrüßung zeigt „Willkommen!" (Large), „Ofen-Controller" (Medium), „Firmware 0.1.0" und „PCB REV DEV" (Small, Muted, zwei Zeilen), jeweils zentriert; bei Fühlerfehler zusätzlich „Fühlerfehler E101" (Error) bei y = 292 … 319, der beim Verschwinden des Fehlers gelöscht wird
- [X] T030 [P] [US1] Host-Tests für `UiStateMachine` (Teil US1) in `firmware/test/test_ui/test_ui_state_machine.cpp`: Startzustand `Boot`; `Boot` + `DisplayReady` → `Welcome`; jede andere Kombination aus `Boot` und alle Ereignisse außer den in US2 definierten aus `Welcome` werden abgewiesen, ändern den Zustand nicht und erzeugen `DEBUG` `msg="transition rejected"`

### Implementation for User Story 1

- [X] T031 [P] [US1] `firmware/lib/temperature/include/`: `temperature_reading.h` (`TemperatureReading { ErrorCode status; Celsius hotJunction; Celsius coldJunction; }`), `oven_temperature.h` (`OvenTemperature { bool valid; Celsius temperature; ErrorCode error; }`), `i_thermocouple_bus.h`, `i_temperature_sensor.h` laut [contracts/interfaces.md § temperature](contracts/interfaces.md#temperature)
- [X] T032 [US1] `firmware/lib/temperature/include/max31855_decoder.h` + `src/max31855_decoder.cpp`: reine Funktion `static TemperatureReading decode(uint32_t raw)` laut [data-model.md § TemperatureReading](data-model.md#temperaturereading-ofentemperatur) – macht T026 grün
- [X] T033 [US1] `firmware/lib/temperature/include/oven_temperature_monitor.h` + `src/oven_temperature_monitor.cpp`: `OvenTemperatureMonitor(IThermocoupleBus&, ILogger&)` implementiert `ITemperatureSensor` mit Plausibilitätsbereich, Entprellung und Timeout aus `app_config.h`; loggt Fehlerwechsel – macht T027 grün
- [X] T034 [P] [US1] Fake `firmware/test/fakes/fake_thermocouple_bus.h` (Warteschlange vorgegebener Rohwörter)
- [X] T035 [P] [US1] `firmware/lib/network/include/`: `wifi_state.h` (`enum class WifiState : uint8_t { NotConfigured, Disconnected, Connected }`), `wall_clock_time.h` (`WallClockTime { bool valid; uint16_t year; uint8_t month, day, hour, minute; }`), `i_network_status.h`, `i_wall_clock.h` laut Contract; Fakes `firmware/test/fakes/fake_network_status.h`, `fake_wall_clock.h`
- [X] T036 [P] [US1] HAL `firmware/lib/hal_esp32/esp32_max31855_bus.{h,cpp}`: `SPIClass`-Transaktion (4 MHz, Modus 0, MSB first) mit CS `kPinMax31855Cs`, liest 4 Bytes zu `uint32_t`; teilt den Bus mit dem Display über `SPI.beginTransaction`/`endTransaction`
- [X] T037 [P] [US1] HAL `firmware/lib/hal_esp32/esp32_wifi_station.{h,cpp}` implementiert `INetworkStatus`: bindet `wifi_secrets.h` per `__has_include` ein; fehlt die Datei oder ist `kWifiSsid` leer → `NotConfigured`, loggt `WARN mod=wifi msg="not configured"` und startet kein WLAN; sonst `WiFi.mode(WIFI_STA)`, `setAutoReconnect(true)`, `WiFi.begin()` ohne Warten; `WiFi.onEvent` setzt einen `std::atomic<WifiState>` (Connected bei `ARDUINO_EVENT_WIFI_STA_GOT_IP`, Disconnected bei `ARDUINO_EVENT_WIFI_STA_DISCONNECTED`); Logzeilen `msg="connected" rssi=<dBm>` bzw. `msg="disconnected" reason=<grund>` werden aus der Hauptschleife geschrieben (Event-Task loggt nicht selbst)
- [X] T038 [P] [US1] HAL `firmware/lib/hal_esp32/esp32_sntp_clock.{h,cpp}` implementiert `IWallClock`: `configTzTime(kTimeZonePosix, kNtpServer1, kNtpServer2)` in `begin()`; Sync-Callback (`sntp_set_time_sync_notification_cb`) setzt atomares Flag; `now()` liefert `valid = true` nur bei gesetztem Flag **und** Jahr ≥ `kMinValidYear`; loggt einmalig `mod=time msg="synced" local="<ISO-Zeit>"`
- [X] T039 [US1] `firmware/lib/ui/include/system_status.h` (`SystemStatus { WifiState wifi; WallClockTime clock; OvenTemperature oven; }`), `firmware/lib/ui/include/firmware_identity.h` (`FirmwareIdentity { const char* version; const char* pcbRevision; }`) und `status_formatter.{h,cpp}` – macht T028 grün
- [X] T040 [US1] `firmware/lib/ui/include/status_bar.h` + `src/status_bar.cpp` und `welcome_screen.{h,cpp}`: zeichnen über `IGraphics`, merken sich den zuletzt gezeichneten Text je Bereich (feste `char`-Puffer), Fühlerfehler-Zeile unten – macht T029 grün
- [X] T041 [US1] `firmware/lib/ui/include/ui_state_machine.h` + `src/ui_state_machine.cpp`: `enum class UiScreen : uint8_t { Boot, Welcome, MainMenu }`, `enum class UiEvent : uint8_t { DisplayReady, WelcomeTimeout, ButtonPressed, RotateCw, RotateCcw }`, zentrale `constexpr`-Übergangstabelle (Von, Ereignis, Nach); zunächst nur `Boot` + `DisplayReady` → `Welcome`; `bool handle(UiEvent)` gibt `false` zurück und loggt `DEBUG` bei abgewiesenem Übergang – macht T030 grün
- [X] T042 [US1] `firmware/lib/app/include/app_controller.h` + `src/app_controller.cpp`: Konstruktor laut [contracts/interfaces.md § app](contracts/interfaces.md#app) (in US1 noch ohne `IRotaryInput`); `setup()` zeichnet nach `DisplayReady` die Begrüßung, ruft danach `INetworkStatus::begin()` und `IWallClock::begin()` auf; registriert im `Scheduler`: Watchdog füttern (jeder Tick), Temperatur 250 ms, Netzwerk/Uhrzeit 1000 ms, Anzeige 100 ms; `tick(now)` ruft nur den Scheduler auf
- [X] T043 [P] [US1] Host-Test `firmware/test/test_app/test_app_controller_us1.cpp` mit allen Fakes: nach `setup()` ist die Begrüßung gezeichnet, bevor `INetworkStatus::begin()` aufgerufen wird (FR-008); Statuszeile reagiert auf geänderte Fake-Werte innerhalb von 1000 ms; Watchdog wird bei jedem `tick` gefüttert
- [X] T044 [US1] `firmware/src/main.cpp` erweitern: statische Instanzen `Esp32Max31855Bus`, `OvenTemperatureMonitor`, `Esp32WifiStation`, `Esp32SntpClock`, `FirmwareIdentity{kFirmwareVersion, kPcbRevision}`, `AppController`; `loop()` ruft nur `app.tick(clock.now())`
- [X] T045 [P] [US1] HIL-Test `firmware/test/hil/test_hil_max31855/test_main.cpp`: 10 Messungen über `Esp32Max31855Bus` + `Max31855Decoder`; erwartet `status == None` und Ofentemperatur sowie Vergleichsstelle zwischen 10 °C und 40 °C (Raumtemperatur)

**Checkpoint**: US1 vollständig: Begrüßung mit Statuszeile, Temperatur und WLAN live; Abnahme-Schritte 1 und 6–13 aus [quickstart.md](quickstart.md) bestanden (Menü noch nicht vorhanden)

---

## Phase 4: User Story 2 - Übergang ins Hauptmenü (Priority: P2)

**Goal**: Nach 3 s oder Encoder-Druck Hauptmenü mit „Heizen", „Profil", „Setup"; Navigation per Drehencoder; Auswahl ohne Wirkung; Statuszeile bleibt sichtbar

**Independent Test**: Board einschalten, 3 s warten bzw. Encoder drücken → Menü; Drehen bewegt Markierung ohne Umlauf; Drücken ändert nichts, Log `msg="menu select" action=none` (Abnahme [quickstart.md](quickstart.md) Schritte 2–5)

### Tests for User Story 2 (zuerst schreiben, müssen fehlschlagen) ⚠️

- [X] T046 [P] [US2] Host-Tests für `QuadratureDecoder` in `firmware/test/test_input/test_quadrature_decoder.cpp`: alle 16 Übergänge (prevAB, currAB) → −1/0/+1; ungültige Doppelsprünge → 0; eine volle Rastung im Uhrzeigersinn = +4 Schritte, gegen = −4
- [X] T047 [P] [US2] Host-Tests für `ButtonDebouncer` in `firmware/test/test_input/test_button_debouncer.cpp`: Pegel muss 20 ms stabil sein; Prellen < 20 ms erzeugt kein Ereignis; genau ein `Pressed` pro Druck (Flanke gedrückt), Loslassen erzeugt kein Ereignis
- [X] T048 [P] [US2] Host-Tests für `RotaryInput` in `firmware/test/test_input/test_rotary_input.cpp` mit `FakeEncoderPins`: `takeDetents() = +2` → zwei `RotateCw`; `−1` → ein `RotateCcw`; Ereignisse in Reihenfolge; `nextEvent()` liefert `None`, wenn leer
- [X] T049 [P] [US2] Host-Tests für `MainMenu` in `firmware/test/test_ui/test_main_menu.cpp`: Einträge „Heizen", „Profil", „Setup" in dieser Reihenfolge; Cursor startet bei 0; „kein Umlauf am Anfang und Ende (Cursor bleibt stehen)"; Drücken liefert die `MenuEntryId` des markierten Eintrags und ändert den Cursor nicht; maximal `kMaxMenuEntries = 8` Einträge (9. wird mit `false` abgelehnt)
- [X] T050 [P] [US2] Host-Tests für `MenuScreen` in `firmware/test/test_ui/test_screens_menu.cpp` mit `FakeGraphics`: Überschrift „Hauptmenü" (Medium, Muted, y = 56 … 79); Einträge ab y = 84, Eintragshöhe 40 px, volle Breite 240 px, Schrift Large, höchstens 5 sichtbar; markierter Eintrag mit Hintergrund Accent und Text Background; beim Drehen werden nur alter und neuer Eintrag neu gezeichnet; beim Wechsel von `Welcome` wird der Inhaltsbereich unterhalb der Statuszeile einmal gelöscht
- [X] T051 [P] [US2] Host-Tests für die Übergänge US2 in `firmware/test/test_ui/test_ui_state_machine.cpp` ergänzen: `Welcome` + `WelcomeTimeout` → `MainMenu`; `Welcome` + `ButtonPressed` → `MainMenu`; `MainMenu` + `RotateCw`/`RotateCcw`/`ButtonPressed` → `MainMenu`; `Welcome` + `RotateCw`/`RotateCcw` abgewiesen; `MainMenu` + `WelcomeTimeout`/`DisplayReady` abgewiesen; jede Zeile der Tabelle in [data-model.md § UiStateMachine](data-model.md#uistatemachine-anzeigezustand) ist abgedeckt

### Implementation for User Story 2

- [X] T052 [P] [US2] `firmware/lib/input/include/`: `input_event.h` (`enum class InputEvent : uint8_t { None, RotateCw, RotateCcw, Pressed }`), `i_rotary_input.h`, `i_encoder_pins.h` laut Contract; Fake `firmware/test/fakes/fake_encoder_pins.h`, `fake_rotary_input.h`
- [X] T053 [US2] `firmware/lib/input/include/quadrature_decoder.h` (header-only, `constexpr`-Tabelle, `static int8_t step(uint8_t prevAB, uint8_t currAB)`, ISR-tauglich ohne Aufrufe in Flash-Code) – macht T046 grün
- [X] T054 [US2] `firmware/lib/input/include/button_debouncer.h` + `src/button_debouncer.cpp` und `rotary_input.{h,cpp}` (`RotaryInput(IEncoderPins&)`, fester Ringpuffer für 16 Ereignisse) – macht T047/T048 grün
- [X] T055 [US2] HAL `firmware/lib/hal_esp32/esp32_encoder_pins.{h,cpp}` implementiert `IEncoderPins`: `pinMode(..., INPUT_PULLUP)` für A, B, Taster; `attachInterruptArg()` auf A und B (`CHANGE`) mit `this` als Argument; ISR (`IRAM_ATTR`) liest beide Pegel, ruft `QuadratureDecoder::step`, zählt Schritte in `std::atomic<int32_t>`; `takeDetents()` gibt volle Rastungen (Schritte / 4) zurück und behält den Rest; `buttonLevelPressed()` = Pegel LOW
- [X] T056 [US2] `firmware/lib/ui/include/main_menu.h` + `src/main_menu.cpp`: `enum class MenuEntryId : uint8_t { Heat, Profile, Setup }`, `MenuEntry { MenuEntryId id; const char* label; }`, festes Array `kMaxMenuEntries`, `bool add(MenuEntry)`, `moveBy(int8_t)`, `MenuEntryId select() const` – macht T049 grün
- [X] T057 [US2] `firmware/lib/ui/include/menu_screen.h` + `src/menu_screen.cpp`: zeichnet Überschrift und bis zu 5 Einträge über `IGraphics`; Teilaktualisierung beim Cursorwechsel – macht T050 grün
- [X] T058 [US2] `UiStateMachine` in `firmware/lib/ui/src/ui_state_machine.cpp` um die Übergänge aus US2 in der zentralen Tabelle erweitern – macht T051 grün
- [X] T059 [US2] `AppController` in `firmware/lib/app/src/app_controller.cpp` erweitern: Konstruktor nimmt `IRotaryInput&` und `MainMenu&`; Scheduler-Aufgabe 5 ms: `poll()` und alle Ereignisse in `UiEvent` übersetzen; `WelcomeTimeout` nach `kWelcomeDurationMs` seit Anzeige der Begrüßung; beim Eintritt in `MainMenu` `MenuScreen` zeichnen; `ButtonPressed` im Menü loggt `INFO mod=ui msg="menu select" entry=<heat|profile|setup> action=none` und ändert nichts an der Anzeige; Statuszeile bleibt in beiden Bildschirmen aktiv (FR-010)
- [X] T060 [P] [US2] Host-Test `firmware/test/test_app/test_app_controller_us2.cpp`: ohne Eingabe nach 3000 ms Menü; Druck bei 500 ms → sofort Menü; Drehen bewegt Cursor; Auswahl erzeugt Logzeile und keine Zeichenaufrufe außerhalb der Statuszeile; Heizung wird nirgends angesprochen (es existiert kein Heizungs-Interface – per Review bestätigen)
- [X] T061 [US2] `firmware/src/main.cpp` erweitern: statische Instanzen `Esp32EncoderPins`, `RotaryInput`, `MainMenu` mit `{Heat, "Heizen"}`, `{Profile, "Profil"}`, `{Setup, "Setup"}` (Einträge nur hier ergänzen, FR-012), an `AppController` übergeben

**Checkpoint**: US1 und US2 funktionieren; Abnahme-Schritte 1–13 aus [quickstart.md](quickstart.md) bestanden

---

## Phase 5: Polish & Cross-Cutting Concerns

**Purpose**: Dokumentation, Qualitätstore und Abnahme

- [X] T062 [P] `docs/state-machine.md` anlegen: Mermaid-Diagramm und Übergangstabelle der `UiStateMachine` exakt wie in `firmware/lib/ui/src/ui_state_machine.cpp`; Abschnitt „Betriebszustände des Ofens: folgt mit Feature ‚Sichere Temperaturregelung'; derzeit nur Idle, Heizung AUS"
- [X] T063 [P] `docs/compatibility.md` anlegen: Tabelle Firmware-Version ↔ PCB-Revision ↔ Gehäuse-Revision mit Zeile `0.1.0 | DEV (Steckbrett, ESP32-DevKitC V4) | – | HIL: offen`
- [X] T064 [P] `docs/README.md` um die neuen Dokumente `error-codes.md`, `state-machine.md`, `compatibility.md` in der Dokumenttabelle ergänzen
- [X] T065 Doxygen-Vollständigkeit aller öffentlichen Header unter `firmware/lib/*/include/` prüfen (Zweck, Parameter mit Einheit, Rückgabe, Fehlerverhalten) und ergänzen
- [X] T066 Qualitätstore lokal ausführen und Befunde beheben: `pio run -d firmware -e esp32_revdev` ohne Warnungen aus eigenem Code, `pio test -d firmware -e native` grün, `pio check -d firmware -e esp32_revdev` ohne Befunde ≥ medium, `clang-format --dry-run --Werror` sauber
- [X] T067 Build mit `-DOVEN_LOG_LEVEL=0` prüfen (`pio run -d firmware -e esp32_revdev` mit überschriebenem Flag): baut, Fehlercodes erscheinen weiterhin auf dem Display (Prinzip IX)
- [ ] T068 Pinbelegung in `firmware/include/board/board_rev_dev.h` gegen Datenblätter von ESP32-DevKitC V4, TFT-Modul (Controller-Aufdruck ST7789 vs. ILI9341 prüfen), MAX31855-Breakout und Encoder-Modul prüfen; Kommentar auf „pins verified against datasheet: yes (<Datum>)" setzen
- [ ] T069 HIL-Tests ausführen (`pio test -d firmware -e esp32_revdev --filter "hil/*"`) und Abnahme laut [quickstart.md](quickstart.md) Schritte 1–13 durchführen; Ergebnis mit Datum in `docs/compatibility.md` eintragen
- [ ] T070 Selbstprüfung gegen Constitution-Prinzipien I–X (Review laut Governance) und `/speckit-analyze` ausführen; Abweichungen beheben oder in [plan.md](plan.md) begründen

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: keine Abhängigkeiten
- **Foundational (Phase 2)**: nach Setup – blockiert alle User Stories
- **US1 (Phase 3)**: nach Foundational
- **US2 (Phase 4)**: nach Foundational; nutzt `StatusBar`, `UiStateMachine` und `AppController` aus US1 (T040–T042) und erweitert sie
- **Polish (Phase 5)**: nach US1 und US2

### User Story Dependencies

- **US1 (P1)**: unabhängig – liefert allein ein nutzbares Ergebnis (Statusanzeige)
- **US2 (P2)**: Input-Modul, `MainMenu` und `MenuScreen` (T046–T057) sind unabhängig von US1 und können parallel entstehen; die Integration (T058–T061) braucht T040–T044

### Umsetzungsnotizen (Abweichungen von den Task-Beschreibungen)

- **Testordner**: Ein Ordner pro Testprogramm (PlatformIO baut je Ordner ein Programm mit
  eigenem `main`): `firmware/test/test_<name>/test_main.cpp`, z. B. `test_max31855_decoder/`,
  `test_oven_temperature_monitor/`, `test_app_controller_us1/`. Fakes liegen in
  `firmware/test/fakes/` und werden relativ eingebunden (`../fakes/...`).
- **HAL-Struktur**: auch `hal_esp32` hat `include/` und `src/`; Plattformklassen sind in
  `esp32_platform.{h,cpp}` zusammengefasst.
- **MAX31855**: eigene, per Software getaktete Leitungen (SCK 14, SO 34, CS 33) statt
  gemeinsamem SPI-Bus (T007, T036; Begründung in research.md R5).
- **Schnittstellen**: `INetworkStatus` und `IWallClock` haben zusätzlich `poll()` (Logging
  aus der Hauptschleife); `IGraphics::drawText` hat zusätzlich die Boxhöhe `h`;
  `AppController` bekommt `MainMenu&` statt `IClock&` (Contract angepasst).
- **Statusbereich**: WLAN 170 px, Uhrzeit 70 px (Contract angepasst).
- **Build**: Plattform `espressif32@6.12.0`; C++-Flags über `scripts/cxx_flags.py`;
  `lib_ldf_mode = deep+`; `OVEN_LOG_LEVEL` ist im Code mit 3 vorbelegt und wird per
  `PLATFORMIO_BUILD_FLAGS=-DOVEN_LOG_LEVEL=0` überschrieben; cppcheck ignoriert `.pio/`.
- **Host-Tests lokal**: Auf dem Entwicklungsrechner liefen die Host-Tests mit dem g++ aus
  WSL (Ubuntu) statt mit `pio test -e native`; in der CI läuft `pio test -e native`.

### Within Each Phase

- Tests vor Implementierung; Tests müssen zuerst fehlschlagen (sicherheitsrelevant: T026/T027)
- Interfaces und Werttypen vor Logik, Logik vor HAL, HAL vor Composition Root
- T014 + T015 + T016 → T017; T014 → T019
- T031 → T032 → T033; T035 → T037/T038; T039 → T040; T041 + T040 → T042 → T044
- T052 → T053 → T054 → T055; T056 → T057; T058 + T057 + T054 → T059 → T061

### Parallel Opportunities

- Setup: T003–T009 parallel nach T001/T002
- Foundational: Tests T010–T013 parallel; T014, T015, T018, T020, T021, T024, T025 parallel
- US1: Tests T026–T030 parallel; T031, T034, T035, T036, T037, T038 parallel
- US2: Tests T046–T051 parallel; T052 parallel zu US1-Implementierung
- Polish: T062–T064 parallel

---

## Parallel Example: User Story 1

```bash
# Tests für US1 gemeinsam anlegen (müssen fehlschlagen):
Task: "TDD-Tests für Max31855Decoder in firmware/test/test_temperature/test_max31855_decoder.cpp"
Task: "TDD-Tests für OvenTemperatureMonitor in firmware/test/test_temperature/test_oven_temperature_monitor.cpp"
Task: "Host-Tests für StatusFormatter in firmware/test/test_ui/test_status_formatter.cpp"

# HAL-Implementierungen gemeinsam:
Task: "HAL Esp32Max31855Bus in firmware/lib/hal_esp32/esp32_max31855_bus.{h,cpp}"
Task: "HAL Esp32WifiStation in firmware/lib/hal_esp32/esp32_wifi_station.{h,cpp}"
Task: "HAL Esp32SntpClock in firmware/lib/hal_esp32/esp32_sntp_clock.{h,cpp}"
```

## Parallel Example: User Story 2

```bash
Task: "Host-Tests für QuadratureDecoder in firmware/test/test_input/test_quadrature_decoder.cpp"
Task: "Host-Tests für ButtonDebouncer in firmware/test/test_input/test_button_debouncer.cpp"
Task: "Host-Tests für MainMenu in firmware/test/test_ui/test_main_menu.cpp"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Phase 1 Setup
2. Phase 2 Foundational
3. Phase 3 US1
4. **STOP and VALIDATE**: Abnahme-Schritte 1, 6–13 aus [quickstart.md](quickstart.md); die
   Begrüßung bleibt stehen, bis US2 umgesetzt ist

### Incremental Delivery

1. Setup + Foundational → Board bootet, loggt, Display an
2. US1 → Statusanzeige (MVP)
3. US2 → Hauptmenü
4. Polish → Dokumentation, Qualitätstore, HIL, Release `0.1.0`

---

## Notes

- [P] = andere Dateien, keine offenen Abhängigkeiten
- Nach jeder Task oder logischen Gruppe committen
- Keine Heizungsausgänge anlegen oder initialisieren (FR-014)
- Keine dynamische Allokation nach `setup()`; kein `delay()` außerhalb von HIL-Tests
- Pinbelegung vor dem ersten Verdrahten prüfen (T068 kann vorgezogen werden)
