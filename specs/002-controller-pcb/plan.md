# Implementation Plan: Controller-Platine (PCB REV A)

**Branch**: `002-controller-pcb` (Feature-Kennung; gearbeitet wird auf `main`) | **Date**: 2026-10-25, ergänzt 2026-10-26 (Platinenlüfter, Netzabdeckung) | **Spec**: [spec.md](spec.md)

**Input**: Feature specification from `specs/002-controller-pcb/spec.md`

## Summary

Erste eigene Platine des Ofen-Controllers (PCB REV A) plus Inbetriebnahme-Firmware. Die
Platine trägt ein ESP32-S3-WROOM-1U-N8 (natives USB, externe Antenne), ein Netzteilmodul
Mean Well IRM-10-5 mit LDO 3,3 V, zwei Thermoelement-Kanäle mit MAX31856 (oben/unten), die
Hardware-Sicherheitskette (Freigabe-Flipflop mit vorrangigem Löschen durch Stopp-Öffner,
TC6501, FAULT der MAX31856 und Watchdog/Spannungsüberwachung TPS3828) sowie die Ansteuerung
von zwei externen Panel-SSRs und einem Sicherheitsrelais. Encoder sitzt auf einem eigenen
kleinen Träger in der Front. Ein Platinenlüfter (Noctua NF-A4x10 5V PWM in der Gehäusewand)
wird über PWM nach der Platinentemperatur geregelt und über sein Drehzahlsignal überwacht
([research.md R18](research.md#r18--platinenlüfter-fr-036-bis-fr-038)). Der Netzbereich bekommt
eine verschraubte Ober- und Unterschale aus PC (3D-Druck), die bei eingestecktem Netzstecker
Messungen im Kleinspannungsbereich erlaubt ([research.md R19](research.md#r19--netzabdeckung-fr-039-sc-010)).

Der Schaltplan entsteht als Code mit SKiDL und wird als Netzliste in KiCad 10 eingelesen;
Layout, ERC/DRC und Fertigungsdaten in KiCad 10 bzw. `kicad-cli`. Die Firmware bekommt eine
zweite Zielumgebung `esp32s3_reva`, Board-Konfiguration REV A, Treiber für die neuen Bausteine
und HIL-Tests für Sensorlesung und jede Abschaltquelle.

## Technical Context

**Language/Version**: Firmware C++17 (`gnu++17`, ohne Exceptions/RTTI); Schaltplan Python 3.13
mit SKiDL

**Primary Dependencies**: KiCad 10.0.3 (`D:\hobby\Elektronik\KiCad\10.0`, `kicad-cli`),
SKiDL (aktuelle Version aus PyPI, in `hardware/.venv`), PlatformIO `espressif32@6.12.0`
(Arduino-ESP32 2.0.17, unterstützt ESP32-S3), LovyanGFX 1.1.16, Unity

**Storage**: Keiner (keine persistenten Daten in diesem Feature)

**Testing**: Host-Tests (`pio test -e native`) für neue Logik; HIL-Tests
(`pio test -e esp32s3_reva`) auf der bestückten Platine; ERC/DRC per `kicad-cli`; elektrische
Abnahme laut [quickstart.md](quickstart.md)

**Target Platform**: PCB REV A mit ESP32-S3-WROOM-1U-N8; REV DEV (ESP32-DevKitC V4) bleibt
baubar

**Project Type**: Hardware (KiCad-Projekt unter `hardware/`) + Embedded-Firmware (`firmware/`)

**Performance Goals**: Last stromlos ≤ 100 ms nach Stopp (Hardwarepfad), ≤ 2,6 s nach
ausbleibendem Watchdog-Signal; Fühlerfehler ≤ 2 s erkannt; Lüfterausfall ≤ 5 s erkannt; Temperatur beider Kanäle ≥ 1×/s

**Constraints**: Platine ≤ 150 × 150 mm (Ziel ≤ 100 × 100 mm), Bauteilhöhe ≤ 40 mm, 2 Lagen,
SMD einseitig, ≥ 0603/SOT-23, keine BGA; Bauteile ≥ 70 °C Betriebstemperatur; keine Testpunkte
im 230-V-Bereich; Laststrom nicht über die Platine; Kleinspannungs-GND nicht an PE;
Netzbereich unter einer Abdeckung (Prüfdraht 1 mm, Spalte ≤ 0,5 mm konstruktiv, Bauteile
darunter ≤ 25 mm hoch, Befestigung ≥ 6 mm vom Netzkupfer)

**Scale/Scope**: 2 Platinen (Hauptplatine, Encoder-Träger), ca. 120 Bauteile, 1 neue
Board-Revision, ca. 10 neue Firmware-Interfaces, 9 neue Fehlercodes

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| Prinzip | Bewertung | Umsetzung in diesem Feature |
|---|---|---|
| I. Modulare Architektur | ✅ | Neue Module `heater` (Heizausgänge, Freigabe, Stopp, externer Watchdog) und `board` (Revisionserkennung, Platinentemperatur) sowie `cooling` (Lüfterkennlinie und -überwachung) als Bibliotheken mit `include/`/`src/`; Treiber nur in `hal_esp32`; Verdrahtung in `main.cpp`. Abhängigkeiten zyklenfrei, siehe [contracts/interfaces.md](contracts/interfaces.md). |
| II. SOLID | ✅ | `OvenTemperatureMonitor` hängt künftig von `IThermocoupleReader` statt vom MAX31855-Rohwort ab (Open/Closed: neuer Wandler = neue Implementierung). Schmale Interfaces: `IHeaterOutputs`, `IEnableLatch`, `IStopButton`, `IExternalWatchdog`, `IRevisionStrap`, `IFanOutput`, `IFanTachometer`. |
| III. HAL & Testbarkeit | ✅ | Decoder, Revisionserkennung, Platinentemperatur-Überwachung, Watchdog-Lebenszeichen, Lüfterkennlinie und Lüfterüberwachung sind host-testbar und werden testgetrieben entwickelt (sicherheitsrelevant). HIL: Sensorlesung beider Kanäle und Abschalten über jede Löschquelle; Ergebnis in `docs/compatibility.md`. |
| IVa. Thermische Sicherheit | ✅ | Heizausgänge mit Pull-down, Freigabe nach Start gelöscht, Löschen vorrangig; Stopp-Öffner speist die Freigabe (Kabelbruch = Stopp); externer Watchdog + interner Watchdog + Brownout; PCB-Temperatur über CJ der MAX31856 (Begründung der Platzierung in [research.md R5](research.md#r5--platinentemperatur-über-die-vergleichsstelle)), Firmware-Grenze 60 °C, Hardware-Schwelle 65 °C (TC6501); Kammer-Übertemperatur per FAULT in Hardware; Sicherheitsrelais gegen kurzgeschlossene SSRs; Thermosicherung bleibt. Lüfterausfall → DISARM und verriegelnder Fehler E206; ohne Firmware (Reset, Absturz) läuft der Lüfter mit voller Drehzahl. Max. Heizdauer und Thermal-Runaway: Feature „Sichere Temperaturregelung". |
| IVb. Elektrische Sicherheit | ✅ | Zertifiziertes, isolierendes Netzteilmodul; Kriech-/Luftstrecken und Fräsung laut [research.md R2](research.md#r2--netzeingang-netzteil-und-isolation); eigene Sicherung + Varistor; Laststrom über Panel-SSRs und Relais außerhalb der Platine; Netzabdeckung als zusätzlicher Berührungsschutz (IPXXD) mit Kunststoffschrauben, die Kriech- und Luftstrecken bleiben die Isolierung ([research.md R19](research.md#r19--netzabdeckung-fr-039-sc-010)); SSR- und Relais-Reserve in [research.md R8](research.md#r8--lastansteuerung-ssr-und-sicherheitsrelais); Montagelöcher ohne Kupfer (GND nicht an PE). Die Firmware setzt nur voraus, was im KiCad-Projekt dokumentiert ist. |
| V. Ressourcen & Determinismus | ✅ | Neue Treiber statisch instanziiert, nicht blockierend (MAX31856 im Automatikmodus, nur Register lesen); USB-CDC-Log ohne Blockieren ([research.md R3](research.md#r3--controller-usb-und-antenne)). |
| VI. Revisionskonsistenz | ✅ | `REV A` im Silkscreen; Revisionserkennung zur Laufzeit per Spannungsteiler am ADC; Firmware passt Laufzeit-Revision gegen Compile-Zeit-Board ab, unbekannt → sicherer Zustand + E204; Pins nur in `board_rev_a.h`; Kompatibilitätsmatrix wird ergänzt; Firmware 0.2.0. |
| VII. Zentrale Konfiguration | ✅ | Grenzwerte (60 °C, Watchdog-Periode, Schwelle der Kammer-FAULT für die Inbetriebnahme, Lüfterkennlinie und Ausfallschwelle) als `constexpr` in `app_config.h`; Pins/Teiler/Encoder-Schritte in `board_rev_a.h`. Keine persistenten Daten. |
| VIII. Zustandsmaschine | ✅ (Teilumfang) | Die Freigabe ist ein Hardware-Speicher, kein Firmware-Zustand. Die Betriebszustände des Ofens kommen mit „Sichere Temperaturregelung"; bis dahin bleibt der Ofen in „Idle, Heizung AUS". Die Inbetriebnahme-Firmware setzt die Freigabe nur in HIL-Tests. |
| IX. Diagnose & Fehlercodes | ✅ | Neue Codes E106–E108 und E201–E206 zentral in `error_code.h`, dokumentiert in `docs/error-codes.md`; Log-Ereignisse für Freigabe, Löschquellen, Revision. |
| X. Gehäuse | ✅ (Schnittstelle) | Platine liefert Maße, Bohrungen, Höhen und Anschlusslage an das Feature „Controller-Gehäuse" (STEP-Export). Aktive Lüftung: 40-mm-Lüfter in der Gehäusewand, Frischluft über die Platine; Lüfterausschnitt, Gitter und Abluftöffnungen legt das Gehäuse-Feature fest. Netzabdeckung aus PC (Kunststoff im Elektronikbereich, Glasübergang ≥ 110 °C) als Fusion-360-Modell, gegen den STEP-Export der Platine geprüft; ihr Bauraum (ca. 27 mm über, 6 mm unter der Platine, 3 mm Überstand links und unten) geht an das Gehäuse-Feature. Die Wärmeabnahme bei max. Ofentemperatur gehört zum Gehäuse-Feature. |
| Technologie-Stack | ✅ | KiCad-Projekt unter `hardware/` mit Schaltplan-Quelle, Layout, Footprints, BOM; PlatformIO-Umgebung `esp32s3_reva` zusätzlich zu `esp32_revdev` und `native`; CI baut beide Zielumgebungen. |

**Ergebnis**: Gate bestanden, keine Abweichungen.

**Re-Check nach Phase 1**: Bestanden. Das Design in [data-model.md](data-model.md),
[contracts/interfaces.md](contracts/interfaces.md) und [contracts/connectors.md](contracts/connectors.md)
hält Modulgrenzen ein; jede Löschquelle wirkt ohne Firmware; alle sicherheitsrelevanten
Logikteile sind host-testbar; die einzige Interface-Änderung (`OvenTemperatureMonitor`) ist
durch Feature 001 abgedeckte Funktionalität und wird mit dessen Tests nachgezogen. Ergänzung
2026-10-26 (Lüfter): erneut bestanden; der Lüfter hängt nicht in der Hardware-Löschkette,
die Platinen-Übertemperatur (TC6501, 65 °C) schützt unabhängig von ihm. Ergänzung
Netzabdeckung: erneut bestanden; keine leitfähigen Teile an der Oberfläche, Wärme des
IRM-10-5 unter der Abdeckung wird gemessen (quickstart 4a).

## Project Structure

### Documentation (this feature)

```text
specs/002-controller-pcb/
├── plan.md              # Dieser Plan
├── research.md          # Phase 0: Entscheidungen R1–R19
├── data-model.md        # Phase 1: Entitäten, Fehlercodes, Freigabe-Logik
├── quickstart.md        # Phase 1: Bring-up, HIL, Abnahme
├── contracts/
│   ├── interfaces.md    # Neue/geänderte Firmware-Schnittstellen
│   └── connectors.md    # Steckverbinder, Pinbelegung REV A, Testpunkte
├── checklists/
│   └── requirements.md
└── tasks.md             # Phase 2 (/speckit-tasks)
```

### Source Code (repository root)

```text
hardware/
├── README.md                         # Werkzeugkette, Build-Befehle, Prüfschritte
├── requirements.txt                  # SKiDL (Version fixiert)
├── lib/                              # projekteigene Symbole/Footprints (.kicad_sym, .pretty),
│                                     #   u. a. oven:Netzabdeckung_REVA (COV1)
├── controller/                       # Hauptplatine REV A
│   ├── schematic/                    # SKiDL-Quellen, eine Datei je Block
│   │   ├── main.py                   #   Einstieg, erzeugt controller.net
│   │   ├── power.py  mcu.py  usb.py  thermocouple.py
│   │   ├── safety.py  load.py  ui.py  misc.py
│   ├── controller.kicad_pro / .kicad_pcb
│   ├── cover/                        # Netzabdeckung: Fusion-Quelle, STEP/STL oben und unten, README
│   ├── controller.net                # erzeugt (versioniert, Review-Grundlage)
│   └── output/                       # Gerber, Bohrdaten, BOM, Bestückungsplan, STEP (erzeugt)
└── encoder-carrier/                  # Encoder-Träger REV A (gleiche Struktur, klein)
firmware/
├── platformio.ini                    # + Umgebung esp32s3_reva
├── include/
│   ├── app_config.h                  # + PCB-Grenze, Watchdog-Periode, Bring-up-Schwellen
│   ├── app_version.h                 # 0.2.0
│   └── board/
│       ├── board_rev_dev.h           # + kEncoderStepsPerDetent bleibt 4
│       └── board_rev_a.h             # neu: Pins, Teiler, SPI-Takte, Schritte/Rastung = 2
├── src/main.cpp                      # Composition Root je Board (#if OVEN_BOARD_REV_*)
├── lib/
│   ├── temperature/                  # + IThermocoupleReader, Max31855Reader, Max31856Decoder
│   ├── heater/                       # neu: IHeaterOutputs, IEnableLatch, IStopButton,
│   │                                 #   IExternalWatchdog, SafetyHeartbeat
│   ├── board/                        # neu: IRevisionStrap, RevisionDetector, PcbTemperatureMonitor
│   ├── cooling/                      # neu: IFanOutput, IFanTachometer, FanCurve, FanMonitor
│   ├── diagnostics/                  # + E106–E108, E201–E206
│   └── hal_esp32/                    # + Max31856 Register-Bus, Heizausgänge, Freigabe, Stopp,
│                                     #   externer Watchdog, ADC-Revision, USB-CDC-Ausgabe,
│                                     #   Lüfter-PWM (LEDC) und Drehzahl (PCNT)
└── test/
    ├── test_temperature/  test_heater/  test_board/  test_cooling/   # native
    └── hil/
        ├── test_hil_max31856/        # beide Kanäle, Fehler, CJ
        ├── test_hil_enable_chain/    # Freigabe, DISARM, Watchdog, FAULT, manuelle Quellen
        ├── test_hil_revision/        # Revisionserkennung
        └── test_hil_fan/             # Drehzahl, Kennlinie, Ausfall (Lüfter festhalten/abziehen)
docs/
├── hardware-umbau.md                 # Pinbelegung REV A (+ CLR_SENSE), Verweise auf Plan
├── werkzeuge-pcb.md                  # Entscheidung SKiDL + KiCad 10
├── error-codes.md                    # + neue Codes
└── compatibility.md                  # + 0.2.0 ↔ REV A
```

**Structure Decision**: `hardware/` nach Verfassung, ein KiCad-Projekt je Platine. Der
Schaltplan ist Python-Code (SKiDL), die erzeugte Netzliste wird mitversioniert, damit Reviews
ohne Python möglich sind. Das Layout entsteht in KiCad 10 aus der Netzliste
(„Platine aus Netzliste aktualisieren"). Firmware-Module wie in Feature 001; neue Hardware nur
über Interfaces.

## Arbeitsablauf Hardware

1. Symbole/Footprints prüfen bzw. anlegen (`hardware/lib`), jede Pinbelegung gegen das
   Datenblatt (Regel aus `docs/werkzeuge-pcb.md`).
2. Schaltplan als SKiDL-Code je Block, ERC in SKiDL, Netzliste erzeugen.
3. Netzliste in KiCad 10 einlesen, Platinenumriss, Zonen (Netz, Kleinspannung), Platzierung
   nach [research.md R13](research.md#r13--layout-lagen-zonen-und-platzierung), Routing.
4. DRC mit Projektregeln (Kriech-/Luftstrecken als Netzklassen-Regeln), `kicad-cli pcb drc`.
5. Exporte per `kicad-cli`: Gerber, Bohrdaten, BOM, Bestückungsplan, STEP für das Gehäuse.
   Netzabdeckung in Fusion 360 gegen den STEP-Export konstruieren, drucken, mit dem Prüfdraht
   abnehmen ([research.md R19](research.md#r19--netzabdeckung-fr-039-sc-010)).
6. Review gegen Spec, Plan und Verfassung (`/speckit-checklist` für IVa/IVb), dann Bestellung.

## Dokumentation, die mit diesem Feature entsteht oder geändert wird

- `hardware/README.md` – Werkzeugkette und Befehle (neu)
- `docs/werkzeuge-pcb.md` – Entscheidung SKiDL + KiCad 10.0.3
- `docs/hardware-umbau.md` – Pinbelegung REV A (+ CLR_SENSE an GPIO 36), endgültige Bauteile
- `docs/error-codes.md` – E106–E108, E201–E206
- `docs/compatibility.md` – Zeile 0.2.0 ↔ REV A mit HIL-Ergebnis
- `docs/state-machine.md` – Hinweis: Freigabe ist Hardware-Speicher, Abschnitt Inbetriebnahme

## Complexity Tracking

Keine Verstöße gegen die Constitution; Tabelle entfällt.
