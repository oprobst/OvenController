---

description: "Task list for 002-controller-pcb"
---

# Tasks: Controller-Platine (PCB REV A)

**Input**: Design documents from `specs/002-controller-pcb/`

**Prerequisites**: [plan.md](plan.md), [spec.md](spec.md), [research.md](research.md),
[data-model.md](data-model.md), [contracts/](contracts/), [quickstart.md](quickstart.md)

**Tests**: Erforderlich. Die Verfassung (Prinzip III) verlangt Host-Tests je Modul,
testgetriebene Entwicklung für Sicherheitslogik und HIL-Tests je PCB-Revision; die Spec
fordert HIL-Tests für Sensorlesung und jede Löschquelle (FR-032). Tests für Sicherheitslogik
werden zuerst geschrieben und müssen fehlschlagen, bevor implementiert wird.

**Organization**: Die Platine wird als Ganzes entworfen und gefertigt. Schaltplan, Layout und
Fertigung liegen deshalb in Phase 2 (Foundational) und blockieren alle Stories. Die
User-Story-Phasen enthalten Firmware, Inbetriebnahme und Abnahme der jeweiligen Funktion.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Parallel möglich (andere Dateien, keine offenen Abhängigkeiten)
- **[Story]**: US1–US4 aus spec.md
- Pfade relativ zum Repository-Root. `kicad-cli` =
  `D:\hobby\Elektronik\KiCad\10.0\bin\kicad-cli.exe`

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Werkzeugkette für Hardware und Zielumgebung der Firmware

- [X] T001 Verzeichnisstruktur `hardware/` laut plan.md anlegen (`lib/`, `controller/schematic/`, `controller/output/`, `encoder-carrier/schematic/`) und `hardware/README.md` schreiben: Werkzeugkette SKiDL + KiCad 10.0.3, Pfad von `kicad-cli`, Befehle für Netzliste, DRC (`kicad-cli pcb drc --exit-code-violations`), Gerber/Bohrdaten/BOM/Bestückungsplan/STEP-Export, Regel „jede Pinbelegung gegen das Datenblatt prüfen"
- [X] T002 `hardware/requirements.txt` mit fixierter SKiDL-Version anlegen; virtuelle Umgebung `hardware/.venv` (Python 3.13) erstellen und installieren; `.gitignore` um `hardware/.venv/`, `*-backups/`, `fp-info-cache`, `*.lck` ergänzen
- [X] T003 Spike SKiDL ↔ KiCad 10 in `hardware/spike/skidl_spike.py`: Symbolpfad auf `D:\hobby\Elektronik\KiCad\10.0\share\kicad\symbols` setzen, einen Widerstand (`Device:R`) und das ESP32-S3-Modul aus der KiCad-10-Bibliothek instanziieren, Netzliste erzeugen, in eine leere Platine in KiCad 10 einlesen (Footprints müssen erscheinen). Ergebnis und ggf. nötige Werkzeugkennung (KICAD8/9) in `specs/002-controller-pcb/research.md` R1 eintragen; scheitert das Lesen der Standardsymbole, Entscheidung „nur projekteigene Symbole" dort festhalten. Spike-Verzeichnis danach löschen
- [X] T004 [P] Umgebung `esp32s3_reva` in `firmware/platformio.ini`: `platform = espressif32@6.12.0`, `board = esp32-s3-devkitc-1`, `board_upload.flash_size = 8MB`, `board_build.partitions = default_8MB.csv`, `build_flags` mit `${env.build_flags}`, `-DOVEN_BOARD_REV_A`, `-DARDUINO_USB_MODE=1`, `-DARDUINO_USB_CDC_ON_BOOT=1`, `lib_deps = lovyan03/LovyanGFX@1.1.16`, `test_filter = hil/*`. Der CI-Schritt für `esp32s3_reva` kommt mit T044 dazu: Bis dahin scheitert der Build an der HAL von REV DEV (Encoder-ISR nur GPIO 0–31, MAX31855-Bus, `VSPI_HOST`), die T040–T044 umbauen
- [X] T005 [P] `firmware/include/board/board_rev_a.h` mit allen Pins und Werten aus `specs/002-controller-pcb/contracts/connectors.md` (Pinbelegung ESP32-S3), `kPcbRevision = "A"`, `kEncoderStepsPerDetent = 2`, `kTftSpiHz = 26666666`, `kThermocoupleSpiHz = 1000000`, Revisionsfenster `kStrapMinMillivolts = 1450`, `kStrapMaxMillivolts = 1850`; Kommentar „pins verified against datasheet: no"; Auswahl des Board-Headers per `OVEN_BOARD_REV_A`/`OVEN_BOARD_REV_DEV` an einer Stelle (`firmware/include/board/board.h`)

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Bauteilprüfung, Schaltplan, Layout, Fertigung der Platine; Firmware-Grundlagen.

**⚠️ CRITICAL**: Ohne bestückte Platine kann keine User Story abgenommen werden.

### Bauteilprüfung (Datenblätter)

- [X] T006 TPS3828-33 laut Datenblatt prüfen: Watchdog-Zeit min/typ/max, Verhalten bei hochohmigem WDI (TI SLVA145), interner Pull-up an /MR, Mindestpulsbreite /MR. Ergebnis in `specs/002-controller-pcb/research.md` R7; liegt die maximale Watchdog-Zeit über 2 s, SC-002 in `specs/002-controller-pcb/spec.md` auf „Maximalwert + 0,1 s" anpassen und Clarifications ergänzen
- [X] T007 [P] Finder 40.61 mit 5-V-DC-Spule prüfen: Bestellnummer, Spulenstrom, Ansprechspannung (muss ≤ 4,3 V sein, weil die 5-V-Schiene ca. 4,5 V hat), Fassung 95.05 + Freilaufmodul; SSR-Eingangsstrom bei 4,5 V. Ergebnis in `specs/002-controller-pcb/research.md` R8/R14
- [X] T008 [P] Konkrete Bauteile mit Bestellnummer, Quelle und Preis auswählen und in `specs/002-controller-pcb/research.md` (Stückliste) sowie in der Komponententabelle von `docs/hardware-umbau.md` eintragen: USB-C-Buchse (handlötbar, ≥ 85 °C), Mini-TC-Printbuchse Typ K, passiver Summer 5 V (≥ 70 °C), Sicherungshalter + T 500 mA, Varistor S07K275, Klemmen 5,08 mm 2-pol. und 3,81 mm 6-pol. (steckbar), IDC 2×5 und 2×4 (Platine und Kabel), Taster 6 × 6 mm, LED, AP7361C-33E, SN74LVC74A, TPS3828-33, TC6501P065, MAX31856, U.FL-Pigtail mit RP-SMA, Antenne; Elkos nur 105-°C-Typen (research R17)
- [X] T009 Symbole und Footprints in `hardware/lib/oven.kicad_sym` und `hardware/lib/oven.pretty/` für alle Teile, die in KiCad 10 fehlen oder abweichen (mindestens IRM-10-5, TC6501, Mini-TC-Buchse, USB-C-Buchse aus T008); jede Pinbelegung gegen das Datenblatt geprüft und in `hardware/lib/README.md` als Liste „Bauteil – Datenblatt-Version – geprüft am" dokumentiert

### Schaltplan Hauptplatine (SKiDL, ein Modul je Block)

- [X] T010 [P] `hardware/controller/schematic/power.py`: J1 (L, N), Sicherung T 500 mA, Varistor S07K275 hinter der Sicherung, IRM-10-5, SS34 vom Netzteilausgang und SS34 von VBUS auf +5V (Dioden-ODER), AP7361C-33 mit 10 µF Ein/Aus, Testpunkte 5V/3V3/GND; Netzklasse `Mains` für L, N und Primärseite (research R2, R14)
- [X] T011 [P] `hardware/controller/schematic/mcu.py`: ESP32-S3-WROOM-1U-N8 mit 10 µF + 100 nF + 100 µF (105 °C), EN-RC 10 kΩ/1 µF, Taster BOOT (GPIO 0) und RESET (EN), Diagnose-LED an GPIO 2, Revisionsteiler 10 kΩ/10 kΩ 1 % an GPIO 1, J9 Debug-UART (GND, GPIO 43, GPIO 44) (research R3, R11, R12)
- [X] T012 [P] `hardware/controller/schematic/usb.py`: USB-C J7, CC1/CC2 je 5,1 kΩ, USBLC6-2SC6 an D+/D−/VBUS, D−/D+ an GPIO 19/20, Schirm über Bestückungsoption 0 Ω bzw. 1 MΩ ∥ 4,7 nF an GND (research R3)
- [X] T013 [P] `hardware/controller/schematic/thermocouple.py`: 2 × MAX31856 (oben: CS GPIO 16, unten: CS GPIO 17, je 10 kΩ Pull-up), gemeinsamer Bus MISO 13 / SCK 14 / MOSI 15, J5/J6 Omega PCC-SMP (`oven:Omega_PCC-SMP_Horizontal`, pad 1 = +) mit Eingangsfilter laut Datenblatt, FAULT beider Bausteine je über MBR0530 (Kathode am FAULT-Pin) auf Netz `CLR`, DRDY unbeschaltet (research R4, R6)
- [X] T014 [P] `hardware/controller/schematic/safety.py`: SN74LVC74A (D=3V3, PRE=3V3, CLK=ARM GPIO 6 mit 100 kΩ Pull-down, CLR=`CLR`, zweites Flipflop mit festen Pegeln), `CLR` mit 100 kΩ nach GND, 1 nF nach GND, 4,7 kΩ von `STOP_NC_RET`; `STOP_NC_OUT` an 3V3; TPS3828-33 (WDI GPIO 21 mit 2,2 kΩ Pull-down, Pflicht laut T006; /MR GPIO 18; /RESET auf `CLR`; 100 nF an VDD), TC6501P065 (HYST an VCC, Ausgang auf `CLR`), `CLR_SENSE` über 10 kΩ an GPIO 36, Q an GPIO 47; Testpunkte LATCH_Q, CLR, ARM, WDI (research R6, R7)
- [X] T015 [P] `hardware/controller/schematic/load.py`: AO3401A High-Side von +5V auf `+5V_SSR` mit 100 kΩ Gate–Source, Gate über BSS138 von Q nach GND; IRLML6346 für SSR_TOP− (Gate GPIO 4) und SSR_BOT− (Gate GPIO 5) je 100 Ω + 100 kΩ Pull-down; Relais-Treiber IRLML6346 von Q mit MBR0530; J2 laut contracts/connectors.md; Testpunkte HEAT_TOP, HEAT_BOT, +5V_SSR (research R8)
- [X] T016 [P] `hardware/controller/schematic/ui.py`: J3 Display (33 Ω an SCK GPIO 12 und MOSI GPIO 11, CS GPIO 10 mit 10 kΩ Pull-up, DC 9, RST 8, Beleuchtung über AO3401A an 3V3 mit 100 kΩ Gate-Pull-up, PWM GPIO 7); J4 Bedienteil mit 10 kΩ Pull-up + 10 kΩ Serie + 10 nF für ENC_A 39, ENC_B 40, ENC_SW 41, STOP_NO 42; zwei USBLC6-2SC6 (VBUS-Pin an 3V3, je zwei Leitungen) für die vier Signale, drittes USBLC6-2SC6 für STOP_NC_OUT/STOP_NC_RET; Summer über IRLML6346 an GPIO 35 mit Freilaufdiode und 100 kΩ Pull-down (research R9–R11)
- [X] T017 `hardware/controller/schematic/main.py`: Blöcke T010–T016 zusammenführen, Netznamen wie in contracts/connectors.md, SKiDL-ERC ohne Fehler, `hardware/controller/controller.net` erzeugen; Abgleich der GPIO-Zuordnung gegen `firmware/include/board/board_rev_a.h` (Skript oder Checkliste in `hardware/README.md`)
- [X] T018 [P] `hardware/encoder-carrier/schematic/main.py`: Alps EC11E15244G1, IDC 2×4 mit Belegung J4, Klemme bzw. JST-XH für Stopp-Öffner (2 Litzen) und Stopp-Schließer (2 Litzen), Netzliste `hardware/encoder-carrier/encoder-carrier.net`

### Layout und Fertigung

- [X] T019 KiCad-10-Projekt `hardware/controller/controller.kicad_pro` mit 2 Lagen, 1,6 mm, 35 µm; Designregeln in `hardware/controller/controller.kicad_dru`: Netzklasse `Mains` ≥ 6,0 mm Abstand zu allen anderen Netzen, L ↔ N ≥ 3,0 mm, keine Vias im Netzbereich
- [ ] T020 Netzliste einlesen, Platinenumriss (Ziel ≤ 100 × 100 mm, Grenze 150 × 150 mm), Zonen in der Reihenfolge Netz → Schlitz → Versorgung (LDO, TC6501 am Hotspot) → Logik (ESP32-S3 mit U.FL an der Kante) → Messung (MAX31856 + J5/J6 an der Kante zur Heizkammer, gemeinsame Kupferfläche); Frässchlitz ≥ 2 mm unter dem IRM-10-5; alle SMD oben; alle Steckverbinder am Rand gruppiert (research R13) in `hardware/controller/controller.kicad_pcb`
  - Stand 2026-10-25: Netzliste übernommen (`hardware/tools/import_netlist.py`), Vorplatzierung
    per `hardware/tools/place_controller.py`: Umriss 100 × 100 mm, 4 × M3 ohne Kupfer, Frässchlitz
    2 × 34 mm unter dem IRM-10-5, Zonen wie oben. DRC: keine Abstands-, Kriech- oder
    Bohrverstöße; nur offene Verbindungen (Routing T021) und Beschriftung an der Kante.
    Offen: Feinplatzierung in KiCad, Omega-Buchse am Musterteil nachmessen, Lage der
    Montagelöcher mit dem Gehäuse abstimmen, Wärmeabstand J5/U5 zum IRM-10-5 prüfen.
- [ ] T071 Schaltplan in KiCad übernehmen und ab dann als Quelle nutzen (Entscheidung 2026-10-25, research R1):
  - Erledigt: SKiDL-Netzlisten als `controller.ref.net` / `encoder-carrier.ref.net` eingefroren; Vorlage aus SKiDL erzeugt (Blätter Netzteil, MCU, USB, Thermoelemente, Sicherheit, Last, Bedienung; Träger: Encoder); Generatorfehler behoben (ENC_B_RAW/ENC_SW_RAW kurzgeschlossen, Zeichensatz der MPN-Felder, fehlende Labels BOOT, LED_DIAG, L_FUSED); `hardware/tools/check_schematic.py` mit 0 Fehlern für beide Platinen; Platine per `import_netlist.py --relink-by-ref` neu verknüpft (Platzierung unverändert), DRC `--schematic-parity` ohne Abweichung; SKiDL-Code nach `hardware/archive/skidl/`
  - Gewollte Änderung 2026-10-26: Eingangsfilter nach MAX31856-Datenblatt (C14/C17 100 nF, C27–C30 10 nF nach GND), Referenznetzliste neu exportiert; C27–C30 liegen auf der Platine noch unterhalb des Umrisses und kommen bei der Feinplatzierung (T020) an U5/U6
  - Offen (Benutzer, in KiCad): lose No-Connect-Markierung auf dem Blatt Thermoelemente (92,7/105,4 mm) entfernen, Blätter lesbar anordnen, Labels wo sinnvoll durch Drähte ersetzen, Symbole aus der Bibliothek aktualisieren (`lib_symbol_mismatch`), `PWR_FLAG` an L_IN/N_IN, L_FUSED, +5V und GND (Träger: GND), ERC-Ausnahme mit Begründung für SDO↔SDO der beiden MAX31856 (Tristate), optional Labels für die übrigen unbenannten Netze (FAULT_*, LED_DIAG_A, TFT_SCK/MOSI, USB_CC1/2, USB_SHIELD)
  - Fertig, wenn: `check_schematic.py` für beide Platinen 0 Fehler, `kicad-cli sch erc` ohne Fehler, DRC `--schematic-parity` ohne Abweichung
- [ ] T021 Routing, GND-Fläche unten (nur Kleinspannung), LDO-Kupferfläche ≥ 4 cm², 4 Montagelöcher M3 **ohne Kupfer**, Testpunkte TP1–TP10 laut contracts/connectors.md (keine im Netzbereich), Bestückungsdruck: „REV A", Projektname, „230 V~"-Warnung, Funktion und Pinbelegung an jedem Steckverbinder, Polarität J5/J6 in `hardware/controller/controller.kicad_pcb`
- [ ] T022 `kicad-cli pcb drc --exit-code-violations` ohne Verletzungen; Exporte nach `hardware/controller/output/`: Gerber, Bohrdaten, BOM, Bestückungsplan, STEP; Bauteilhöhe ≤ 40 mm im STEP prüfen
- [ ] T023 [P] Encoder-Träger: Projekt, Layout (ca. 25 × 30 mm, Encoder zentriert, IDC am Rand), DRC und Exporte in `hardware/encoder-carrier/`
- [ ] T024 Sicherheits-Review der Hardware: `/speckit-checklist` gegen Prinzip IVa/IVb ausführen (Ergebnis `specs/002-controller-pcb/checklists/safety.md`), Selbstprüfung gegen Prinzip I–X, Abweichungen beheben
- [ ] T025 Bestellung von Platinen und Bauteilen (Benutzer); Bezugsquellen und Preise in `docs/hardware-umbau.md` nachtragen
- [ ] T026 Bestückung (Benutzer) und stromlose Prüfung nach `specs/002-controller-pcb/quickstart.md` Abschnitt 2; Ergebnis in `docs/compatibility.md` (Spalte Bemerkung)

### Firmware-Grundlagen (parallel zur Hardware)

- [ ] T027 [P] Fehlercodes E106 `ThermocoupleOverUnderVoltage`, E107 `ChamberOverTemperature`, E108 `ColdJunctionOutOfRange`, E201 `PcbOverTemperature`, E202 `PcbTemperatureUnknown`, E203 `EnableLatchMismatch`, E204 `UnsupportedPcbRevision`, E205 `HardwareTrip` in `firmware/lib/diagnostics/include/error_code.h` und mit Bedeutung/Ursache/Abhilfe in `docs/error-codes.md`
- [ ] T028 [P] Konstanten in `firmware/include/app_config.h`: `kPcbLimitCelsius = 60`, `kPcbRecoverCelsius = 55`, `kPcbRecoverHoldMs = 5000`, `kHeartbeatPeriodMs = 250`, `kHeartbeatMaxAgeMs = 500`, `kChamberFaultLimitCelsius = 300` (Inbetriebnahme-Wert), `kLatchMismatchMs = 10`, `kStrapSampleCount = 8`
- [ ] T029 Test zuerst: `firmware/test/test_temperature/` an `IThermocoupleReader` anpassen (Fake `firmware/test/fakes/fake_thermocouple_reader.h`); Tests für `Max31855Reader` (Rohwort → `TemperatureReading` wie bisher) und für `OvenTemperatureMonitor` mit Kanalnamen; `OvenTemperature` bekommt `coldJunction`
- [ ] T030 Refactoring `firmware/lib/temperature/`: `i_thermocouple_reader.h`, `max31855_reader.{h,cpp}`, `OvenTemperatureMonitor(IThermocoupleReader&, ILogger&, const char* channelName)`, `oven_temperature.h` mit `coldJunction`; Composition Root REV DEV in `firmware/src/main.cpp` nachziehen – macht T029 grün, `pio test -e native` und `pio run -e esp32_revdev` grün
- [ ] T031 [P] `firmware/include/app_version.h` auf 0.2.0

**Checkpoint**: Platine bestückt und stromlos geprüft; Firmware-Grundlagen grün auf `native` und `esp32_revdev`.

---

## Phase 3: User Story 1 – Platine in Betrieb nehmen (Priority: P1) 🎯 MVP

**Goal**: Platine startet über USB, zeigt Version und „REV A", Bedienung und beide
Temperaturkanäle funktionieren; Heizausgänge bleiben aus.

**Independent Test**: quickstart.md Abschnitte 3 und 5 (nur USB, dann Netz ohne Last).

### Tests für User Story 1 ⚠️

- [ ] T032 [P] [US1] Host-Tests `firmware/test/test_temperature/test_max31856_decoder.cpp`: Temperatur- und CJ-Umrechnung aus Rohbytes; Fehlerbits laut data-model.md: „OPEN → E101", „OVUV → E106", „TCHIGH → E107", „CJRANGE, CJHIGH, CJLOW → E108", „TCRANGE, TCLOW → E104"
- [ ] T033 [P] [US1] Host-Tests `firmware/test/test_temperature/test_max31856_reader.cpp` mit `firmware/test/fakes/fake_max31856_bus.h`: `configure()` schreibt Typ K, 50 Hz, Automatik, Fühlerbrucherkennung, Interrupt-Fehlermodus, FAULT-Maske und Obergrenze `kChamberFaultLimitCelsius`; Antwort 0x00/0xFF → E105; jede 10. Abfrage Konfigurationsvergleich, Abweichung → E105; `clearFault()` setzt das Quittierbit
- [ ] T034 [P] [US1] Host-Tests `firmware/test/test_board/test_revision_detector.cpp`: „REV A: Fenster 1450–1850 mV", 1449 und 1851 mV → `Unknown`; kein Teiler → `Dev`; erkannt ≠ kompiliert oder `Unknown` → E204
- [ ] T035 [P] [US1] Host-Tests für zwei Temperaturen in der Statuszeile in `firmware/test/test_ui/test_status_formatter.cpp` (oben/unten, Fehlercode je Kanal z. B. „E101 oben")

### Implementierung für User Story 1

- [ ] T036 [P] [US1] `firmware/lib/temperature/include/max31856_decoder.h` + `src/max31856_decoder.cpp` – macht T032 grün
- [ ] T037 [US1] `firmware/lib/temperature/include/i_max31856_bus.h`, `max31856_reader.{h,cpp}` laut contracts/interfaces.md – macht T033 grün
- [ ] T038 [P] [US1] Modul `firmware/lib/board/` mit `i_revision_strap.h`, `pcb_revision.h` (`PcbRevisionId`), `revision_detector.{h,cpp}` – macht T034 grün
- [ ] T039 [US1] `firmware/lib/ui/` Statuszeile mit zwei Temperaturen (StatusFormatter, StatusBar, SystemStatus) – macht T035 grün; REV DEV zeigt weiter einen Wert
- [ ] T040 [P] [US1] HAL `firmware/lib/hal_esp32/esp32_max31856_bus.{h,cpp}`: SPI3 über GPIO-Matrix (MISO 13, SCK 14, MOSI 15), Modus 1, `kThermocoupleSpiHz`, CS je Instanz
- [ ] T041 [P] [US1] HAL `firmware/lib/hal_esp32/esp32_revision_strap.{h,cpp}`: ADC1 an GPIO 1, `kStrapSampleCount` Messungen, kalibrierte mV, Mittelwert; REV DEV liefert „kein Teiler"
- [ ] T042 [US1] HAL-Anpassungen für den ESP32-S3 in `firmware/lib/hal_esp32/`: Encoder an GPIO 39–41 ohne interne Pull-ups (externe vorhanden), `kEncoderStepsPerDetent` aus dem Board-Header; LovyanGFX auf SPI2 mit Pins aus `board_rev_a.h` und `kTftSpiHz`; Hintergrundbeleuchtung per LEDC an GPIO 7 (P-MOSFET: LOW = an); serielle Ausgabe über USB-CDC mit Sende-Timeout 0 ms (nie blockierend); Diagnose-LED an GPIO 2 blinkt im Betrieb
- [ ] T043 [US1] Sicherer Start in `firmware/lib/hal_esp32/esp32_heater_outputs.{h,cpp}` und `esp32_enable_latch.{h,cpp}` (Minimalumfang): GPIO 4/5/6 als Ausgang LOW vor allem anderen, DISARM-Puls an GPIO 18 als erste Aktion in `setup()`
- [ ] T044 [US1] Composition Root REV A in `firmware/src/main.cpp` (`#if OVEN_BOARD_REV_A`): sicherer Start (T043), Revisionsprüfung (E204 → Heizausgänge aus, DISARM, Fehleranzeige, kein Menü), zwei `Max31856Reader` mit `configure()`, zwei `OvenTemperatureMonitor` („oben", „unten"), Log `msg="revision"`; danach Ablauf aus Feature 001. `pio run -e esp32s3_reva` muss danach bauen; CI-Schritt „Build firmware (esp32s3_reva)" in `.github/workflows/firmware.yml` ergänzen (aus T004 verschoben)
- [ ] T045 [P] [US1] HIL-Test `firmware/test/hil/test_hil_revision/test_main.cpp`: Teilerspannung im Fenster REV A, Erkennung = A
- [ ] T046 [P] [US1] HIL-Test `firmware/test/hil/test_hil_max31856/test_main.cpp`: Konfiguration beider Kanäle bestätigt, Kammertemperatur und CJ bei Raumtemperatur plausibel (10–40 °C), Kanäle ≤ 2 °C auseinander, abgezogener Fühler (Testanweisung im Log) → E101 ≤ 2 s
- [ ] T047 [US1] Abnahme nach `specs/002-controller-pcb/quickstart.md` Abschnitte 3 und 5 durchführen, Ergebnis in `docs/compatibility.md`

**Checkpoint**: REV A läuft mit Feature-001-Funktionen, beide Kanäle, sicherer Start.

---

## Phase 4: User Story 2 – Hardware-Sicherheitskette (Priority: P1)

**Goal**: Freigabe nur nach Benutzeraktion; jede Löschquelle schaltet ohne Firmware ab; die
Firmware erkennt und meldet Abschaltungen.

**Independent Test**: quickstart.md Abschnitt 4 (HIL automatisch und manuell M1–M4) sowie 6.4
und 6.5 mit Ersatzlast.

### Tests für User Story 2 (zuerst, müssen fehlschlagen) ⚠️

- [ ] T048 [P] [US2] Host-Tests `firmware/test/test_heater/test_safety_heartbeat.cpp` mit `firmware/test/fakes/fake_external_watchdog.h`: WDI wird nur umgeschaltet, „wenn beide Meldungen jünger als 500 ms sind und seit dem letzten Umschalten mindestens 250 ms vergangen sind"; fehlende oder alte Meldung → kein Umschalten
- [ ] T049 [P] [US2] Host-Tests `firmware/test/test_board/test_pcb_temperature_monitor.cpp` mit Fake-Sensoren: Maximum der gültigen CJ-Werte; „celsius ≥ 60 °C → E201"; „kein gültiger Wert → E202"; Meldung erst gelöscht, „wenn der Wert 5 s lang unter 55 °C liegt"
- [ ] T050 [P] [US2] Host-Tests `firmware/test/test_heater/test_enable_monitor.cpp` mit `firmware/test/fakes/fake_enable_latch.h` und `fake_stop_button.h`: E203 bei „`armed` ohne vorheriges ARM seit dem letzten DISARM", „`armed` bleibt nach DISARM länger als 10 ms gesetzt", „`armed` und `clearActive` gleichzeitig länger als 10 ms"; E205 bei Übergang armed → nicht armed ohne eigenes DISARM, Ursache `stop` (stopPressed), `line` (clearActive ohne stopPressed), `fault`/`pcb`/`watchdog` nach Zusatzinfo

### Implementierung für User Story 2

- [ ] T051 [US2] Modul `firmware/lib/heater/` mit `heater_channel.h`, `i_heater_outputs.h`, `i_enable_latch.h`, `i_stop_button.h`, `i_external_watchdog.h` laut contracts/interfaces.md; `EnableMonitor` als neue Klasse in `contracts/interfaces.md` ergänzen (Signatur: `update(Milliseconds now, bool firmwareArmed, bool firmwareDisarmed)`, `ErrorCode status()`, `const char* tripCause()`)
- [ ] T052 [US2] `firmware/lib/heater/src/safety_heartbeat.cpp` – macht T048 grün
- [ ] T053 [US2] `firmware/lib/heater/src/enable_monitor.cpp` – macht T050 grün
- [ ] T054 [US2] `firmware/lib/board/src/pcb_temperature_monitor.cpp` – macht T049 grün
- [ ] T055 [P] [US2] HAL vervollständigen: `firmware/lib/hal_esp32/esp32_enable_latch.{h,cpp}` (ARM: steigende Flanke an GPIO 6; DISARM: LOW-Puls 100 µs an GPIO 18 (Datenblatt: ≥ 1 µs); `isArmed()` GPIO 47; `isClearActive()` GPIO 36 LOW), `esp32_stop_button.{h,cpp}` (GPIO 42, LOW = gedrückt, entprellt), `esp32_external_watchdog.{h,cpp}` (GPIO 21 umschalten)
- [ ] T056 [US2] Integration in `firmware/lib/app/` und `firmware/src/main.cpp`: Scheduler-Aufgaben Herzschlag (jeder Durchlauf, `SafetyHeartbeat::tick`), Platinentemperatur 250 ms, Freigabeüberwachung 10 ms; jede Meldung E1xx/E201–E205 → `allOff()`, `disarm()`, Log und Fehleranzeige mit Code; Normalbetrieb ruft `arm()` nie auf
- [ ] T057 [US2] HIL-Test `firmware/test/hil/test_hil_enable_chain/test_main.cpp`: Q nach Start gelöscht; ARM setzt Q; DISARM löscht ≤ 1 ms; Herzschlag stoppen → Q gelöscht ≤ Watchdog-Maximalzeit aus T006; Kammer-Obergrenze unter Raumtemperatur setzen → Q gelöscht und `clearActive`; ARM bei aktivem Stopp wirkungslos; manuelle Schritte M1–M4 mit Testanweisung im Log und Prüfung von Q (quickstart Abschnitt 4)
- [ ] T058 [US2] Lastteil mit Ersatzlast anschließen und quickstart 6.4 (≤ 100 ms nach Stopp) und 6.5 (simuliertes SSR-Kurzschluss → Relais trennt) durchführen; Ergebnis in `docs/compatibility.md`
- [ ] T059 [US2] `docs/state-machine.md` um Abschnitt „Hardware-Freigabe" ergänzen (Mermaid aus data-model.md, Hinweis: Hardware-Speicher, Firmware setzt nur in HIL-Tests)

**Checkpoint**: Jede Löschquelle aus FR-021 nachweislich wirksam; SC-001–SC-003 belegt.

---

## Phase 5: User Story 3 – Ober- und Unterhitze getrennt schalten (Priority: P2)

**Goal**: Beide Heizkreise unabhängig taktbar, nur bei gesetzter Freigabe.

**Independent Test**: quickstart.md 6.1–6.3 und 6.6 mit Ersatzlast.

- [ ] T060 [P] [US3] HAL `firmware/lib/hal_esp32/esp32_heater_outputs.{h,cpp}` vervollständigen: `set(HeaterChannel, bool)` an GPIO 4/5, `allOff()`; Host-Fake `firmware/test/fakes/fake_heater_outputs.h`
- [ ] T061 [US3] HIL-Test `firmware/test/hil/test_hil_heater_outputs/test_main.cpp`: Freigabe setzen, nur oben, nur unten, beide im Sekundentakt 60 s, Freigabe gelöscht + Heizbefehl → keine Last (Prüfung durch Benutzer per Testanweisung im Log)
- [ ] T062 [US3] quickstart 6.1–6.3 und 6.6 (100 × Einschalten, 100 × Neustart mit Logikanalysator an den SSR-Eingängen) durchführen; Ergebnis in `docs/compatibility.md`

**Checkpoint**: SC-004 belegt; Grundlage für „Sichere Temperaturregelung".

---

## Phase 6: User Story 4 – Einbau, Wartung und Aktualisierung (Priority: P3)

**Goal**: Platine im Gehäuse, WLAN mit externer Antenne, USB von außen, ohne Löten tauschbar.

**Independent Test**: quickstart.md Abschnitt 7.

- [ ] T063 [P] [US4] Übergabe an das Gehäuse-Feature: Maßzeichnung (Außenmaße, Montagelöcher, Steckverbinderlage, höchstes Bauteil) als Abschnitt „Übergabe Platine REV A" in `docs/hardware-umbau.md`, STEP aus `hardware/controller/output/` und `hardware/encoder-carrier/output/`
- [ ] T064 [P] [US4] Montage von Antennen-Pigtail (RP-SMA-Durchführung) und Panel-USB-Verlängerung in `docs/hardware-umbau.md` beschreiben (Lochdurchmesser, Lage, Kabellängen)
- [ ] T065 [US4] quickstart Abschnitt 7 durchführen (WLAN bei geschlossenem Gehäuse, Upload über die Wandbuchse, Aus- und Einbau < 10 min); Ergebnis in `docs/compatibility.md`

**Checkpoint**: SC-007 und SC-008 belegt.

---

## Phase 7: Polish & Cross-Cutting Concerns

- [ ] T066 [P] `docs/compatibility.md`: Zeile Firmware 0.2.0 ↔ PCB REV A ↔ Gehäuse „–" mit HIL- und Abnahmeergebnis; Zeile 0.2.0 ↔ REV DEV (weiterhin unterstützt)
- [ ] T067 [P] `firmware/include/board/board_rev_a.h`: Pins gegen Datenblätter (ESP32-S3, MAX31856, TPS3828, SN74LVC74A) und KiCad-Netzliste geprüft, Kommentar „pins verified against datasheet: yes (<Datum>)"
- [ ] T068 [P] `docs/hardware-umbau.md`: Komponententabelle mit endgültigen Bauteilen, Bezugsquellen, Preisen; Status „REV A gefertigt"
- [ ] T069 `pio test -e native`, `pio run -e esp32_revdev`, `pio run -e esp32s3_reva`, `pio check`, clang-format; ohne neue Warnungen (Qualitätstore der Verfassung)
- [ ] T070 Abschluss-Review: Selbstprüfung gegen Prinzip I–X, `/code-review` für die Firmware-Änderungen, `/speckit-analyze` für Spec/Plan/Tasks; Abweichungen beheben oder begründen

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: keine; T003 (Spike) vor T009–T018.
- **Foundational (Phase 2)**: T006–T009 vor dem Schaltplan; T010–T016 parallel; T017 nach
  T010–T016; T019–T022 nacheinander nach T017; T024 vor T025; T026 nach Lieferung.
  Firmware-Grundlagen T027–T031 parallel zur Hardware.
- **US1 (Phase 3)**: Tests und Implementierung ab Ende der Firmware-Grundlagen; HIL und Abnahme
  (T045–T047) erst nach T026.
- **US2 (Phase 4)**: nach US1 (nutzt Start, Messkanäle und Board-Modul); T058 braucht das Lastteil.
- **US3 (Phase 5)**: nach US2 (Heizen nur mit funktionierender Freigabe).
- **US4 (Phase 6)**: T063/T064 nach T022; T065 nach US1.
- **Polish**: nach den gewünschten Stories.

### Within Each User Story

- Tests zuerst, müssen fehlschlagen; dann Logik, HAL, Integration, HIL, Abnahme.

### Parallel Opportunities

- Phase 1: T004 und T005 parallel zu T001–T003.
- Phase 2: T007/T008 parallel; Schaltplanblöcke T010–T016 parallel; T018 und T023 parallel zur
  Hauptplatine; T027–T031 parallel zur gesamten Hardware.
- US1: T032–T035 parallel; T036, T038, T040, T041 parallel; T045/T046 parallel.
- US2: T048–T050 parallel; T055 parallel zu T052–T054.

---

## Parallel Example: Schaltplanblöcke (Phase 2)

```text
Task: "power.py – Netzeingang, IRM-10-5, Dioden-ODER, LDO (T010)"
Task: "mcu.py – ESP32-S3, Reset/Boot, LED, Revisionsteiler (T011)"
Task: "thermocouple.py – 2 × MAX31856, Buchsen, FAULT auf CLR (T013)"
Task: "safety.py – Flipflop, Löschleitung, TPS3828, TC6501 (T014)"
```

## Parallel Example: User Story 1

```text
Task: "Host-Tests Max31856Decoder (T032)"
Task: "Host-Tests Max31856Reader (T033)"
Task: "Host-Tests RevisionDetector (T034)"
Task: "Host-Tests Statuszeile mit zwei Temperaturen (T035)"
```

---

## Implementation Strategy

### MVP (User Story 1)

1. Phase 1 und 2: Werkzeugkette, Schaltplan, Layout, Fertigung; parallel Firmware-Grundlagen.
2. Phase 3: Platine startet sicher, misst beide Kanäle, bedienbar.
3. **STOP und prüfen**: quickstart Abschnitte 3 und 5.

### Incremental Delivery

1. US1 → Platine läuft wie das Steckbrett, nur besser (MVP).
2. US2 → Sicherheitskette nachgewiesen; erst danach Last anschließen.
3. US3 → Heizkreise schaltbar; Voraussetzung für „Sichere Temperaturregelung".
4. US4 → Einbau ins Gehäuse.

---

## Notes

- Hardware-Tasks T025/T026 und manuelle Prüfungen erledigt der Benutzer; die übrigen Tasks
  sind so beschrieben, dass sie ohne weiteren Kontext umsetzbar sind.
- Jede Pinbelegung gegen das Datenblatt prüfen (`docs/werkzeuge-pcb.md`).
- Netzspannung nur nach quickstart-Sicherheitshinweis; vor US2 keine Last anschließen.
- Commit nach jeder Task oder logischen Gruppe.
