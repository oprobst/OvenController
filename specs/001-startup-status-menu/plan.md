# Implementation Plan: Startbildschirm mit Statusanzeige und Hauptmenü

**Branch**: `001-startup-status-menu` | **Date**: 2026-10-24 | **Spec**: [spec.md](spec.md)

**Input**: Feature specification from `specs/001-startup-status-menu/spec.md`

## Summary

Erste Firmware für den Ofen-Controller: Nach dem Einschalten zeigt ein ESP32-DevKitC V4 auf
einem 2,8"-TFT (ST7789, 240×320, Hochformat) eine Begrüßung mit Firmware-Version und PCB-Revision sowie
eine Statuszeile (WLAN-Zustand, Uhrzeit/Datum per SNTP, Ofentemperatur über MAX31855). Nach
3 s oder Encoder-Druck folgt ein Hauptmenü mit den Platzhaltern „Heizen", „Profil", „Setup".
Die Heizung wird nicht angesteuert.

Weil dies das erste Feature ist, legt der Plan auch das Firmware-Grundgerüst fest:
PlatformIO-Projekt mit modularen Bibliotheken unter `firmware/lib/`, HAL für alle
Hardwarezugriffe, Host-Tests (`native`), Logging, Fehlercodes, Watchdog und CI.

## Technical Context

**Language/Version**: C++17 (`gnu++17`), ohne Exceptions und RTTI

**Primary Dependencies**: PlatformIO-Plattform `espressif32@6.12.0` (Arduino-ESP32 2.0.17),
LovyanGFX 1.1.16 (Display), Unity (Tests). MAX31855, Encoder, Logger: eigene Module.

**Storage**: Keiner. WLAN-Zugangsdaten zur Compile-Zeit aus `wifi_secrets.h` (nicht
versioniert), siehe [research.md R7](research.md#r7--wlan-verbindung-und-zugangsdaten)

**Testing**: Unity in der Umgebung `native` (Host) mit handgeschriebenen Fakes;
HIL-Tests mit `pio test -e esp32_revdev`

**Target Platform**: ESP32-DevKitC V4 (ESP32-WROOM-32E/D), Steckbrett-Aufbau „PCB REV DEV"

**Project Type**: Embedded-Firmware (PlatformIO-Projekt unter `firmware/`)

**Performance Goals**: Begrüßung ≤ 2 s nach Einschalten; Menü bedienbar ≤ 5 s; Temperatur
mindestens 1×/s aktualisiert; Fühlerfehler ≤ 2 s sichtbar (geplant ≤ 0,5 s); Encoder ohne
verlorene Rastungen

**Constraints**: Keine dynamische Allokation nach der Initialisierung, nicht-blockierende
Hauptschleife, kein Vollbild-Framebuffer (RAM), Watchdog 5 s, Heizung in keinem Zustand an

**Scale/Scope**: 2 Bildschirme (Begrüßung, Hauptmenü), 3 Menüeinträge, 1 Sensor, 1 Board-
Revision

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| Prinzip | Bewertung | Umsetzung in diesem Feature |
|---|---|---|
| I. Modulare Architektur | ✅ | Module als PlatformIO-Bibliotheken unter `firmware/lib/<modul>/{include,src}`; nur Interfaces sind öffentlich; Verdrahtung ausschließlich in `src/main.cpp` (Composition Root); keine globalen veränderlichen Zustände außerhalb davon. Abhängigkeitsgraph siehe [contracts/interfaces.md](contracts/interfaces.md) – zyklenfrei. |
| II. SOLID | ✅ | Schmale Rollen-Interfaces (`ITemperatureSensor`, `INetworkStatus`, `IClock`, `IWallClock`, `IRotaryInput`, `IGraphics`, `ILogger`, `IWatchdog`); Konstruktor-Injektion; neue Menüeinträge über `MenuEntry`-Liste ohne Änderung der Menüführung (FR-012). |
| III. HAL & Testbarkeit | ✅ | Alle Arduino-/ESP-IDF-Aufrufe in `lib/hal_esp32`; Logik in host-testbaren Modulen; Unity-Tests je Modul; HIL-Tests für Sensorlesung und Watchdog. Sicherheitsrelevante Logik (Fühler-Fehlererkennung) wird testgetrieben entwickelt. HIL „sicheres Abschalten der Heizung" entfällt, da keine Heizung angesteuert wird – folgt im Heiz-Feature. |
| IVa. Thermische Sicherheit | ✅ (eingeschränkt anwendbar) | Keine Heizungsausgänge konfiguriert → sicherer Zustand dauerhaft. Fühlerfehler werden erkannt und mit Code angezeigt. Watchdog und Brownout aktiv. Start immer ohne Heizung. Max.-Temperatur-Überwachung, Thermal-Runaway, Höchstlaufzeit, PCB-Temperatursensor und Hardware-Pull-down gehören zu den Features „Sichere Temperaturregelung" und „Controller-Platine", weil es hier keinen Heizpfad gibt. |
| IVb. Elektrische Sicherheit | ✅ (nicht anwendbar) | Steckbrett-Aufbau ausschließlich mit 5 V über USB; keine Netzspannung, keine Last. |
| V. Ressourcen & Determinismus | ✅ | Statische Objekte im Composition Root; LovyanGFX ohne Sprites; kooperativer Scheduler mit festen Perioden; kein `delay()`; Einheiten im Typ (`Celsius`, `Milliseconds`). |
| VI. Revisionskonsistenz | ✅ | Revision `DEV` zur Compile-Zeit (Umgebung `esp32_revdev`); Anzeige von Firmware-Version (SemVer `0.1.0`) und Revision auf der Begrüßung und im Log; `docs/compatibility.md` wird angelegt. |
| VII. Zentrale Konfiguration | ✅ | `firmware/include/app_config.h` (Zeiten, Grenzen) und `firmware/include/board/board_rev_dev.h` (Pins, SPI-Takte), alles `constexpr`. Keine persistenten Daten. |
| VIII. Zustandsmaschine | ✅ (Teilumfang) | Bildschirmablauf als explizite Zustandsmaschine `UiStateMachine` (`enum class UiScreen`, Übergangstabelle), dokumentiert in `docs/state-machine.md`. Die Betriebszustände des Ofens (Idle, Heizen, Fehler …) werden mit dem Heiz-Feature ergänzt; hier existiert nur „Idle, Heizung AUS". Ein Fühlerfehler führt deshalb noch nicht in einen verriegelnden Fehlerzustand, sondern wird in der Statuszeile angezeigt. |
| IX. Diagnose & Fehlercodes | ✅ | `ILogger` mit `key=value`-Format, Level per Build-Flag, komplett abschaltbar; nicht-blockierend. `enum class ErrorCode` zentral in `lib/diagnostics`; `docs/error-codes.md` mit E101–E105. Fehlercode in der UI auch ohne Logging. |
| X. Gehäuse | ✅ (nicht anwendbar) | Kein Gehäuse in diesem Feature. |
| Technologie-Stack | ✅ | PlatformIO mit Zielumgebung + `native`; `.clang-format`; cppcheck via `pio check`; GitHub-Actions-CI; Englisch im Code, Deutsch in der Doku; Interfaces mit Präfix `I` und Doxygen. |

**Ergebnis**: Gate bestanden. Keine Abweichungen, die begründet werden müssen; die als
„eingeschränkt anwendbar" markierten Punkte betreffen Hardware (Heizpfad, Platine), die in
diesem Feature nicht existiert.

**Re-Check nach Phase 1**: Bestanden. Das Design in [data-model.md](data-model.md) und
[contracts/](contracts/) hält die Modulgrenzen ein; Abhängigkeiten zeigen nur auf
Interfaces; die Fehlererkennung des Fühlers ist rein logisch und host-testbar.

## Project Structure

### Documentation (this feature)

```text
specs/001-startup-status-menu/
├── plan.md              # Dieser Plan
├── research.md          # Phase 0: Entscheidungen R1–R13
├── data-model.md        # Phase 1: Entitäten, Fehlercodes, Zustandsmaschine
├── quickstart.md        # Phase 1: Aufbau, Build, Test, Abnahme
├── contracts/
│   ├── interfaces.md    # Modul-Schnittstellen und Abhängigkeiten
│   ├── ui-screens.md    # Bildschirmaufbau und Texte
│   └── log-format.md    # Format der seriellen Log-Zeilen
├── checklists/
│   └── requirements.md
└── tasks.md             # Phase 2 (/speckit-tasks)
```

### Source Code (repository root)

```text
.clang-format
.github/workflows/firmware.yml        # CI: Build, native-Tests, pio check, clang-format
docs/
├── compatibility.md                  # Firmware ↔ PCB ↔ Gehäuse (neu)
├── error-codes.md                    # Fehlercodes (neu)
└── state-machine.md                  # Zustandsmaschinen (neu)
firmware/
├── platformio.ini                    # Umgebungen esp32_revdev, native
├── include/
│   ├── app_config.h                  # zentrale constexpr-Konstanten
│   ├── app_version.h                 # Firmware-Version (SemVer)
│   ├── wifi_secrets.example.h        # Muster, versioniert
│   ├── wifi_secrets.h                # lokal, .gitignore
│   └── board/
│       └── board_rev_dev.h           # Pins und Hardwareparameter REV DEV
├── src/
│   └── main.cpp                      # Composition Root
├── lib/
│   ├── units/                        # Celsius, Milliseconds (header-only)
│   ├── diagnostics/                  # ErrorCode, ILogger, LogFormatter, Logger, NullLogger
│   ├── platform/                     # IClock, IWatchdog, ISerialOut, ISystemInfo
│   ├── temperature/                  # ITemperatureSensor, IThermocoupleBus,
│   │                                 #   Max31855Decoder, OvenTemperatureMonitor
│   ├── network/                      # INetworkStatus, IWallClock (Interfaces + Typen)
│   ├── input/                        # IRotaryInput, IEncoderPins, QuadratureDecoder,
│   │                                 #   ButtonDebouncer
│   ├── ui/                           # IGraphics, UiStateMachine, MainMenu, StatusFormatter,
│   │                                 #   WelcomeScreen, MenuScreen, StatusBar
│   ├── app/                          # Scheduler, AppController
│   └── hal_esp32/                    # Implementierungen gegen Arduino/ESP-IDF/LovyanGFX
│                                     #   (nur Zielumgebung, in native per lib_ignore aus)
└── test/
    ├── fakes/                        # Fake-Implementierungen der Interfaces
    ├── test_units/ test_diagnostics/ test_temperature/ test_input/
    ├── test_ui/ test_app/            # native (Host)
    └── hil/
        ├── test_hil_max31855/        # Sensorlesung auf Zielhardware
        └── test_hil_watchdog/        # Neustart bei blockierter Hauptschleife
```

**Structure Decision**: Repository-Struktur laut Constitution (`firmware/`, `docs/`; `hardware/`
und `mechanical/` folgen mit ihren Features). Jedes Modul ist eine PlatformIO-Bibliothek mit
`include/` (öffentliche Interfaces und Werttypen) und `src/` (Implementierung). Die
Logik-Module hängen nur von `units`, `diagnostics` und `platform` ab; `hal_esp32` implementiert
die Interfaces und ist das einzige Modul mit Arduino-/ESP-IDF-Abhängigkeiten.

## Ablauf zur Laufzeit

1. `setup()`: Serial und Logger, Reset-Grund loggen, Watchdog aktivieren, SPI und Display
   initialisieren, Begrüßung zeichnen (Ziel < 1 s nach Reset), dann Sensor, Encoder,
   WLAN-Start (nicht blockierend) und SNTP konfigurieren.
2. `loop()` ruft nur `AppController::tick(now)` auf. Der Scheduler führt periodische Aufgaben
   aus:

   | Aufgabe | Periode |
   |---|---|
   | Watchdog füttern | jeder Durchlauf |
   | Encoder-Taster abtasten, Encoder-Ereignisse abholen | 5 ms |
   | Temperatur lesen und bewerten | 250 ms |
   | WLAN-Status und Uhrzeit abfragen | 1000 ms |
   | Bildschirm aktualisieren (nur geänderte Bereiche) | 100 ms |

3. Die `UiStateMachine` wechselt nach 3000 ms oder bei Encoder-Druck von `Welcome` zu
   `MainMenu`.

## Dokumentation, die mit diesem Feature entsteht oder geändert wird

- `docs/state-machine.md` – Bildschirm-Zustandsmaschine (Mermaid + Übergangstabelle)
- `docs/error-codes.md` – E101–E105
- `docs/compatibility.md` – Firmware 0.1.0 ↔ PCB REV DEV ↔ kein Gehäuse
- `docs/hardware-umbau.md` – Vorauswahl ergänzt: Prototyp-Board ESP32-DevKitC V4,
  Anzeige 2,8" TFT ST7789
- `.gitignore` – `firmware/include/wifi_secrets.h`

## Complexity Tracking

Keine Verstöße gegen die Constitution; Tabelle entfällt.
