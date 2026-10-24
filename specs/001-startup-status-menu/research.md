# Research: Startbildschirm mit Statusanzeige und Hauptmenü

**Feature**: [spec.md](spec.md) | **Plan**: [plan.md](plan.md) | **Datum**: 2026-10-24

Jede Entscheidung folgt dem Schema *Decision / Rationale / Alternatives considered*. Alle
Punkte aus dem Technical Context, die mit NEEDS CLARIFICATION markiert waren, sind hier
aufgelöst.

## R1 – Microcontroller-Board (Prototyp)

- **Decision**: Espressif **ESP32-DevKitC V4** (Modul ESP32-WROOM-32E/32D, 4 MB Flash,
  520 KB SRAM) auf dem Steckbrett. PlatformIO-Board `esp32dev`. Die erste eigene Platine
  (Feature „Controller-Platine") wird auf dasselbe Modul aufsetzen.
- **Rationale**: Vom Benutzer gewählt; verbreitet, günstig, ausreichend RAM für WLAN, TFT und
  Regelung. Die Constitution verlangt die MCU-Wahl im ersten Feature-Plan; ein späterer
  Wechsel betrifft nur die HAL.
- **Alternatives considered**: ESP32-S3-DevKitC-1 (native USB, mehr RAM) – vom Benutzer nicht
  gewählt; ESP32-C3 (zu wenige GPIOs für TFT + Encoder + mehrere SSR).

## R2 – PCB-Revision ohne eigene Platine

- **Decision**: Die Revision wird zur Compile-Zeit über die PlatformIO-Umgebung festgelegt.
  Für den Steckbrett-Aufbau gibt es die Revision **`DEV`** (Umgebung `esp32_revdev`,
  Konfigurationsheader `board_rev_dev.h`). Angezeigt wird „PCB REV DEV".
- **Rationale**: Prinzip VI erlaubt die Festlegung zur Compile-Zeit. Codier-Widerstände gibt
  es erst auf der echten Platine; `REV A` bekommt dann eine eigene Umgebung und einen eigenen
  Header.
- **Alternatives considered**: Laufzeit-Erkennung per Strapping-Widerstand – auf dem
  Steckbrett ohne Mehrwert, folgt mit dem Platinen-Feature.

## R3 – Framework und Toolchain

- **Decision**: PlatformIO, Plattform `espressif32@6.12.0` (Arduino-ESP32 Core 2.0.17,
  ESP-IDF 4.4), C++17 (`-std=gnu++17`). Exceptions und RTTI werden per `build_unflags`
  (`-fexceptions`, `-frtti`) entfernt und durch `-fno-exceptions -fno-rtti` ersetzt. Die
  reinen C++-Flags setzt `firmware/scripts/cxx_flags.py` nur für C++-Quellen (sonst warnt der
  C-Compiler). Abhängigkeiten zwischen den Bibliotheken unter `lib/` findet PlatformIO mit
  `lib_ldf_mode = deep+`.
- **Umsetzung**: 6.12.0 statt der ursprünglich geplanten 6.9.0, weil diese Version bereits
  installiert war; beide bringen denselben Arduino-Core 2.0.17.
- **Rationale**: Arduino-Core bietet stabile WLAN-, SNTP- und SPI-Treiber mit wenig Aufwand;
  der Zugriff bleibt in der HAL (Prinzip III). Version fest gepinnt für reproduzierbare
  Builds.
- **Alternatives considered**: Reines ESP-IDF (mehr Kontrolle, deutlich mehr Boilerplate);
  Arduino-Core 3.x (`pioarduino`-Fork, in PlatformIO noch nicht offiziell).
- **Risiko**: Die vorkompilierten Core-Bibliotheken nutzen intern Exceptions; ein Fehler dort
  führt zu `abort()` → Neustart → sicherer Zustand (Heizung AUS). Akzeptabel.

## R4 – Anzeige: 2,8" TFT ST7789 240×320 (Hochformat) und Grafikbibliothek

- **Decision**: 2,8"-TFT mit ST7789, 240×320, **Hochformat** (Rotation 0), über SPI (VSPI,
  40 MHz). Die Trägerplatine (50 × 81,6 mm) passt nur hochkant ins 52–56 mm breite
  Bedienfeld (`docs/hardware-umbau.md`).
  Grafikbibliothek **LovyanGFX 1.1.16**, gekapselt hinter der Schnittstelle `IGraphics`.
  Kein Vollbild-Framebuffer (240×320×2 = 150 KB wären neben WLAN zu viel); stattdessen werden
  nur geänderte Bereiche neu gezeichnet. Texte werden mit Hintergrundfarbe gezeichnet, damit
  nichts flackert.
- **Schriften**: DejaVu 12/18/24 aus LovyanGFX. Sie enthalten nur ASCII; ä, ö, ü, Ä, Ö, Ü
  werden aus dem Grundbuchstaben plus zwei gezeichneten Punkten zusammengesetzt, ° als kleiner
  Kreis, ß als „ss". Die Unicode-Schriften von LovyanGFX (efont, IPA) wären je Größe mehrere
  hundert KB groß und passen neben WLAN nicht sinnvoll in den Flash.
- **Rationale**: Vom Benutzer gewählt. LovyanGFX unterstützt ST7789 und das ESP32-DMA, wird
  im Code (nicht über globale Makros) konfiguriert und passt damit zu „ein Header pro
  Revision". Ohne Sprites allokiert die Bibliothek nach `init()` keinen Speicher (Prinzip V).
- **Alternatives considered**: TFT_eSPI (Konfiguration über globale Build-Makros, Wartung
  stockt), Adafruit_ST7789 (langsamer, kein DMA), U8g2 (nur für monochrome Displays).
- **Hinweis**: Viele 2,8"-Module tragen tatsächlich einen ILI9341 oder einen Touch-Controller
  XPT2046. Der Controller-Typ wird beim ersten Test gegen den Aufdruck geprüft; ein Wechsel
  betrifft nur die Panel-Konfiguration in der HAL. Touch wird nicht verwendet.

## R5 – Temperaturmessung MAX31855

- **Decision**: MAX31855K-Breakout an **eigenen Leitungen** (SCK, SO, CS), per Software
  getaktet (32 Takte, < 50 µs). Lesezyklus 250 ms.
- **Umsetzung**: Ursprünglich war der gemeinsame SPI-Bus mit dem Display geplant. LovyanGFX
  und die Arduino-`SPIClass` würden sich dann dieselbe Hardware teilen; eigene Leitungen
  vermeiden dieses Risiko und eine Display-Übertragung kann nie eine Messung verzögern. Eigener schlanker Treiber: Die HAL liefert nur das rohe
  32-Bit-Wort; die Auswertung (`Max31855Decoder`) ist reine Logik und auf dem Host testbar.
- **Fehlererkennung** (ergibt „Fühlerfehler" mit Code, siehe [data-model.md](data-model.md)):
  - Fault-Bits des MAX31855: offen (OC), Kurzschluss gegen GND (SCG), gegen VCC (SCV).
  - Rohwort `0x00000000` oder `0xFFFFFFFF` → Sensor antwortet nicht.
  - Plausibilitätsbereich **−20 °C … 350 °C**; außerhalb → unplausibel.
  - Kein gültiger Wert seit 1000 ms → veraltet.
  - Entprellung: 2 fehlerhafte Messungen in Folge setzen den Fehler, 2 gültige heben ihn auf.
    Damit wird ein abgezogener Fühler nach ≤ 500 ms angezeigt (SC-005 verlangt ≤ 2 s).
- **Rationale**: Der MAX31855 ist in `docs/hardware-umbau.md` vorausgewählt. Ein eigener
  Treiber umgeht Bibliotheken mit `delay()` (Adafruit) und macht Fehlerfälle testbar.
- **Alternatives considered**: Adafruit_MAX31855 (blockierende Software-SPI-Option,
  Fehlerbehandlung per `NAN`); MAX31856 (genauer, aber nicht vorausgewählt).
- **Hinweis**: Die Vergleichsstellentemperatur (interner Sensor des MAX31855) wird
  mitgelesen und geloggt. Sie ersetzt **nicht** den von Prinzip IVa geforderten
  PCB-Temperatursensor; der folgt mit dem Platinen- bzw. Heiz-Feature.

## R6 – Drehencoder

- **Decision**: Encoder-Modul (z. B. KY-040) an drei GPIOs mit internen Pull-ups.
  A/B werden per GPIO-Interrupt (beide Flanken) ausgewertet. Die Zustandstabelle des
  Quadratur-Dekoders liegt als reine Logik vor (`QuadratureDecoder`, host-getestet); die ISR
  in der HAL ruft nur diese Tabelle auf und zählt atomar. Die ISR wird per
  `attachInterruptArg()` mit einem Zeiger auf die HAL-Instanz registriert, damit keine
  globale veränderliche Variable nötig ist (Prinzip I). Ein Menüschritt = eine Rastung
  (4 Flanken). Der Taster wird in der Hauptschleife alle 5 ms abgetastet und mit 20 ms
  entprellt (`ButtonDebouncer`, host-getestet).
- **Rationale**: Interrupts verhindern verlorene Schritte, während das Display zeichnet.
- **Alternatives considered**: ESP32-PCNT-Hardware (gut, aber schwerer zu mocken und
  Encoder-abhängige Filterung); reines Polling (verliert Schritte bei langen Zeichenvorgängen).

## R7 – WLAN-Verbindung und Zugangsdaten

- **Decision**: WLAN im Station-Modus, `setAutoReconnect(true)`, Verbindungsaufbau startet
  nach der Anzeige der Begrüßung und läuft im Hintergrund (Event-Task des ESP32). Der Status
  wird über WLAN-Events in atomare Variablen der HAL-Klasse geschrieben und von der
  Hauptschleife jede Sekunde abgefragt; ein Verbindungsverlust ist damit nach ≤ 10 s
  sichtbar.
- **Zugangsdaten**: Header `firmware/include/wifi_secrets.h` mit `constexpr`-Werten, per
  `.gitignore` ausgeschlossen. Ein versioniertes Muster `wifi_secrets.example.h` beschreibt
  das Format. Per `__has_include` baut die Firmware auch ohne die Datei; dann ist der Status
  „nicht eingerichtet". Leerer Netzwerkname = „nicht eingerichtet".
- **Rationale**: Erfüllt FR-013 (lokal, nicht im Repository). Kein persistenter Speicher nötig
  (Prinzip VII entfällt hier). Das spätere Setup-Feature ersetzt die Quelle hinter derselben
  Schnittstelle.
- **Alternatives considered**: PlatformIO-`extra_configs` mit `build_flags` (Passwort mit
  Sonderzeichen schwer zu quoten, landet in Build-Logs); Umgebungsvariablen (für CI-Builds
  unnötig).

## R8 – Uhrzeit und Datum

- **Decision**: SNTP über `configTzTime()` mit der POSIX-Zeitzone
  `CET-1CEST,M3.5.0,M10.5.0/3` und den Servern `de.pool.ntp.org`, `pool.ntp.org`. Gültig ist
  die Zeit erst nach dem SNTP-Sync-Callback **und** wenn das Jahr ≥ 2026 ist. Danach läuft die
  Systemuhr auch ohne WLAN weiter (Edge Case „Verbindung bricht ab").
- **Rationale**: Im ESP-IDF eingebaut, kein Zusatzmodul; Sommer-/Winterzeit automatisch.
  Die Jahresprüfung verhindert die Anzeige von 01.01.1970 (FR-005).
- **Alternatives considered**: Eigene NTP-Implementierung (unnötig); RTC-Baustein (laut
  Annahme nicht vorhanden).

## R9 – Watchdog, Brownout, Neustartgrund

- **Decision**: Task-Watchdog (`esp_task_wdt`, Timeout 5 s, Panic → Neustart) für den
  Arduino-Loop-Task. Brownout-Erkennung bleibt wie im Arduino-Core voreingestellt aktiv
  (`CONFIG_ESP32_BROWNOUT_DET=y`); beim Start wird `esp_reset_reason()` geloggt. Ein
  HIL-Test prüft, dass ein absichtlich blockierter Loop zum Neustart führt.
- **Rationale**: Prinzip IVa verlangt Watchdog und Brownout ab der ersten Firmware.
- **Alternatives considered**: Nur Interrupt-Watchdog (deckt hängende Loop nicht ab).

## R10 – Logging

- **Decision**: Logger-Schnittstelle `ILogger`; Formatierung als reine Logik
  (`LogFormatter`), Ausgabe über `ISerialOut` (HAL, UART0, 115200 Baud, 2 KB TX-Puffer).
  Passt eine Zeile nicht mehr in den Puffer, wird sie verworfen und ein Zähler erhöht – das
  Logging blockiert nie. Format siehe [contracts/log-format.md](contracts/log-format.md).
  Level per Build-Flag `OVEN_LOG_LEVEL`; `OVEN_LOG_LEVEL=0` schaltet Logging komplett ab
  (`NullLogger`).
- **Rationale**: Prinzip IX.
- **Alternatives considered**: `ESP_LOGx` (Format nicht einheitlich, schreibt direkt auf
  UART).

## R11 – Tests

- **Decision**: Unity (PlatformIO-Standard) in der Umgebung `native` für alle Logik-Module;
  Fakes für HAL-Schnittstellen werden von Hand geschrieben (`test/fakes/`). HIL-Tests in
  `test/hil/` laufen mit `pio test -e esp32_revdev`.
- **Rationale**: Ohne zusätzliche Abhängigkeiten; Mock-Bedarf ist gering, weil die
  Schnittstellen schmal sind.
- **Alternatives considered**: GoogleTest/GMock (mächtiger, aber unter Windows aufwendiger zu
  bauen).
- **Hinweis**: Auf dem Entwicklungsrechner ist derzeit kein Host-Compiler installiert.
  `native`-Tests brauchen lokal einen GCC (z. B. MSYS2 MinGW-w64); in der CI (Ubuntu) ist er
  vorhanden. Siehe [quickstart.md](quickstart.md).

## R12 – Code-Stil, statische Analyse, CI

- **Decision**: `.clang-format` (Basis Google, 4 Leerzeichen, 100 Spalten);
  statische Analyse per `pio check` mit cppcheck (`--enable=warning,style,performance`,
  Fehler ab Schwere „medium"); Compiler-Flags `-Wall -Wextra` für eigenen Code.
  GitHub Actions `.github/workflows/firmware.yml`: Build `esp32_revdev`, `pio test -e native`,
  `pio check`, `clang-format --dry-run --Werror`.
- **Rationale**: Technologie-Stack und Qualitätstore der Constitution. Das Repository liegt
  bereits auf GitHub (`oprobst/OvenController`).
- **Alternatives considered**: clang-tidy (unter PlatformIO aufwendiger einzubinden; kann
  später ergänzt werden).

## R13 – Pinbelegung REV DEV (ESP32-DevKitC V4)

| Signal | GPIO | Bemerkung |
|---|---|---|
| TFT SCK | 18 | VSPI, nur TFT |
| TFT MOSI | 23 | VSPI, nur TFT (MISO wird nicht benötigt) |
| TFT CS | 22 | |
| TFT DC | 21 | |
| TFT RST | 4 | |
| TFT Hintergrundbeleuchtung | 32 | an = HIGH |
| MAX31855 SCK | 14 | eigene Leitung, per Software getaktet |
| MAX31855 SO | 34 | reiner Eingang, vom MAX31855 getrieben (kein Pull-up nötig) |
| MAX31855 CS | 33 | |
| Encoder A (CLK) | 25 | interner Pull-up |
| Encoder B (DT) | 26 | interner Pull-up |
| Encoder Taster (SW) | 27 | interner Pull-up, aktiv LOW |
| Stopp-Taste | – | in diesem Feature nicht angeschlossen; Pin und Hardware-Abschaltung folgen mit den Features „Sichere Temperaturregelung" und „Controller-Platine" |

- **Rationale**: Keine Strapping-Pins (0, 2, 5, 12, 15), keine Flash-Pins (6–11), keine
  reinen Eingänge (34–39) für Signale, die einen Pull-up brauchen. GPIO 14 gibt beim Booten
  kurz ein PWM-Signal aus; für den MAX31855 (nur lesend, CS dann inaktiv) ist das harmlos. Die Belegung steht nur in
  `board_rev_dev.h` (Prinzip VII).
- **Regel**: Belegung vor dem Verdrahten gegen das Datenblatt des DevKitC V4 und der Module
  prüfen.
- **Heizungsausgänge** werden in diesem Feature nicht belegt und nicht initialisiert.
