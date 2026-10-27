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
- [X] T072 [P] Platinenlüfter laut Datenblatt prüfen und in `specs/002-controller-pcb/research.md` R18/R17 sowie `hardware/lib/README.md` („Bauteil – Datenblatt-Version – geprüft am“) festhalten: Noctua NF-A4x10 5V PWM (Strom, Betriebstemperatur −10…+70 °C, Drehzahl bei 20 % PWM, Verhalten ohne PWM-Signal = volle Drehzahl), Molex 47053-1000 (Betriebstemperatur ≥ 85 °C, Pin 1 = GND, Abgleich mit `Connector:FanPinHeader_1x04_P2.54mm_Vertical`), ESP32-S3 Datasheet Tabelle 2-2 für GPIO 37/38 (Ergebnis in die Glitch-Tabelle von `docs/hardware-umbau.md`)
  - Stand 2026-10-26: Lüfter max. 0,35 W / 0,07 A, −10…+70 °C; Molex 47053-1000 ≥ 75 °C, KiCad-Footprint passt; GPIO 37/38 ohne Einschalt-Glitch (Tabelle 2-2 nur GPIO 1–20). Einträge in research R17/R18, `hardware/lib/README.md`, `docs/hardware-umbau.md`
- [X] T082 [P] Maße für die Netzabdeckung prüfen und in `specs/002-controller-pcb/research.md` R19 eintragen: Höhe und Pinlage (primär/sekundär) des IRM-10-5 laut Datenblatt (R2: 21,5 mm), Höhe von J1 mit aufgestecktem Stecker MSTB und Kabelbogen, Höhe des Sicherungshalters F1 mit Kappe (alle Teile unter dem Deckel ≤ 25 mm), Außendurchmesser der Mantelleitung H05VV-F 2 × 0,75 mm² am Muster, PC-Filament auswählen (bevorzugt UL 94 V-0, Glasübergang ≥ 110 °C)
  - Stand 2026-10-26: höchstes Teil IRM-10-5 mit 21,5 mm, J1 8,6 mm (Stecker ≤ 15 mm), F1 10,1 mm; Kabel 5,7–7,2 mm je Hersteller, Durchführung nach Messung am gekauften Kabel; Filament PC-FR (V-0) laut research R19. Offen: RV1 und J1 mit Stecker am Muster messen

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
    Montagelöcher mit dem Gehäuse abstimmen, Wärmeabstand J5/U5 zum IRM-10-5 prüfen;
    nach T073 J10 am Rand der Logikzone (Kabel zur Gehäusewand) mit BSS138 und Widerständen
    daneben platzieren.
  - Stand 2026-10-27 (nach T084): Kleinteile (R, C, D, Q, kleine ICs, Testpunkte) per lokaler
    Optimierung um höchstens 2 mm verschoben und in 90°-Schritten gedreht, damit die Luftlinien
    kürzer werden (Summe der Luftlinien dieser Teile 1525 → 1182 mm). Dabei mindestens 0,5 mm
    zwischen den Courtyards (Handbestückung), nichts in den Sperrflächen von COV1. Bestückungsdruck:
    Werte auf F.Fab, Referenzen 0,8 mm (Steckverbinder und große ICs 1 mm) aufrecht neben dem
    Bauteil und frei von Pads und Kante; 19 Referenzen ohne freien Platz nur auf F.Fab (C7, C19,
    C20, C22, C23, C25, C31, C32, J10, Q8, R8, R12, R14, R15, R23, R28, R33, R35, R41); J10 bekommt
    seine Beschriftung „FAN 5V“ mit T021. DRC `--schematic-parity`: nur offene Verbindungen und
    2 Hinweise zur U1-Silkscreen an der Kante (Footprint-Grafik). Offen: Montagelöcher mit dem
    Gehäuse abstimmen, Steckbarkeit J10 neben dem U.FL-Anschluss prüfen.
  - Buchsenwechsel 2026-10-27: J5/J6 von Omega PCC-SMP-K auf RS PRO 381-7564 umgestellt (Preis,
    Einzelbezug, Maßzeichnung im Datenblatt; Footprint `oven:RS_381-7564_Horizontal`, siehe
    `hardware/lib/README.md`). Schaltplanfelder Footprint/MPN/Source geändert,
    `controller.ref.net` neu exportiert (übernimmt dabei auch die Netznamen nach der
    Label-Bereinigung, Verbindungen unverändert: Abgleich 0 Fehler, 0 Hinweise). Auf der
    Platine: Stifte unter U5/U6 (x 62,8 / 82,6), Steckseite 0,6 mm vor der Unterkante, Loch
    Ø 2,7 mm für eine Schraube M2,5; zwischen J5 und J6 jetzt 2,7 mm statt 0,4 mm. Parity ohne
    Abweichung. Offen: Polarität und Maße am Musterteil prüfen, Breite der Stecker
    (Mittenabstand 19,8 mm).
- [X] T071 Schaltplan in KiCad übernehmen und ab dann als Quelle nutzen (Entscheidung 2026-10-25, research R1):
  - Erledigt: SKiDL-Netzlisten als `controller.ref.net` / `encoder-carrier.ref.net` eingefroren; Vorlage aus SKiDL erzeugt (Blätter Netzteil, MCU, USB, Thermoelemente, Sicherheit, Last, Bedienung; Träger: Encoder); Generatorfehler behoben (ENC_B_RAW/ENC_SW_RAW kurzgeschlossen, Zeichensatz der MPN-Felder, fehlende Labels BOOT, LED_DIAG, L_FUSED); `hardware/tools/check_schematic.py` mit 0 Fehlern für beide Platinen; Platine per `import_netlist.py --relink-by-ref` neu verknüpft (Platzierung unverändert), DRC `--schematic-parity` ohne Abweichung; SKiDL-Code nach `hardware/archive/skidl/`
  - Gewollte Änderung 2026-10-26: Eingangsfilter nach MAX31856-Datenblatt (C14/C17 100 nF, C27–C30 10 nF nach GND), Referenznetzliste neu exportiert; C27–C30 liegen auf der Platine noch unterhalb des Umrisses und kommen bei der Feinplatzierung (T020) an U5/U6
  - Umbau 2026-10-26: die 7 Blätter zu 2 zusammengelegt – „Leistung und Sicherheit“ (A3: Netzteil, Sicherheit, Last) und „Steuerung und Messung“ (A2, da Bedienung allein fast A3 füllt: MCU, USB, Thermoelemente, Bedienung); Inhalte nur verschoben, nicht umverdrahtet, je Bereich ein beschrifteter Rahmen; Abgleich 0 Fehler, ERC unverändert, Platine über Referenzen neu verknüpft (Platzierung unverändert), Parity ohne Abweichung
  - Offen (Benutzer, in KiCad): lose No-Connect-Markierung im Bereich Thermoelemente (Blatt Steuerung, 369,6/102,9 mm) entfernen, Blätter lesbar anordnen (ggf. Steuerung nach Verdichten auf A3), Labels wo sinnvoll durch Drähte ersetzen, Symbole aus der Bibliothek aktualisieren (`lib_symbol_mismatch`), `PWR_FLAG` an L_IN/N_IN, L_FUSED, +5V und GND (Träger: GND), ERC-Ausnahme mit Begründung für SDO↔SDO der beiden MAX31856 (Tristate), optional Labels für die übrigen unbenannten Netze (FAULT_*, LED_DIAG_A, TFT_SCK/MOSI, USB_CC1/2, USB_SHIELD)
  - Fertig, wenn: `check_schematic.py` für beide Platinen 0 Fehler, `kicad-cli sch erc` ohne Fehler, DRC `--schematic-parity` ohne Abweichung
  - Abgeschlossen 2026-10-27: Blätter vom Benutzer lesbar angeordnet (Labels in Gruppen nach Verwendung bereinigt, Seitenverweise eingeschaltet); eingebettete Symbole aus der KiCad-10-Bibliothek aktualisiert (abgeleitete Symbole zusammengeführt, 130 × `lib_symbol_mismatch` weg), Drahtreste entfernt, `PWR_FLAG` an N_IN, L_FUSED, +5V und VBUS, ERC-Ausnahme mit Begründung für SDO↔SDO (U5/U6) in `controller.kicad_pro`. Ergebnis: Abgleich 0 Fehler, `kicad-cli sch erc --exit-code-violations` 0 Verstöße, Parity ohne Abweichung
- [X] T073 Platinenlüfter in `hardware/controller/controller_steuerung.kicad_sch` (eigener beschrifteter Rahmen „Lüfter“) nach research R18 und contracts/connectors.md J10 zeichnen (KiCad geschlossen bzw. nach T071): J10 Molex 47053-1000 (1 GND, 2 +5V, 3 FAN_TACH, 4 FAN_PWM) mit 10 µF und 100 nF an +5V; FAN_PWM: GPIO 38 (globales Label `FAN_PWM`) an das Gate eines BSS138 mit 100 kΩ Gate-Pull-down, Source an GND, Drain an J10 Pin 4 mit 1 kΩ Pull-up an +5V; FAN_TACH: J10 Pin 3 mit 10 kΩ Pull-up an 3,3 V, 1 kΩ Serie und 1 nF nach GND an GPIO 37 (globales Label `FAN_TACH`); TP12 an FAN_TACH; MPN/Source-Felder wie bei den übrigen Bauteilen. Danach: `hardware/tools/check_schematic.py` (Tabellen `MODULE_PIN`/`HEADER_TO_NET`) um FAN_PWM ↔ GPIO 38 und FAN_TACH ↔ GPIO 37 erweitern, `firmware/include/board/board_rev_a.h` um `kPinFanTach = 37`, `kPinFanPwm = 38`, `kFanPwmHz = 25000`, `kFanPulsesPerRevolution = 2` ergänzen, `hardware/controller/controller.ref.net` neu exportieren (gewollte Änderung, Grund im Commit), `check_schematic.py controller` mit 0 Fehlern, ERC ohne neue Befunde, Platine per `hardware/tools/import_netlist.py` aktualisieren und DRC `--schematic-parity` ohne Abweichung
  - Stand 2026-10-26: Rahmen „Lüfter" auf dem Blatt Steuerung (rechts unten), Bauteile J10, Q8, R40–R43, C31–C33, TP12 mit Stummeln, Power-Symbolen und Labels (Aufräumen mit Drähten durch den Benutzer); No-Connect an IO37/IO38 von U3 entfernt; Abgleich 129 Teile, 73 Netze, 0 Fehler; ERC ohne neue Befundtypen; Platine: 10 Footprints unterhalb des Umrisses (y = 145/158 mm) für die Feinplatzierung, Parity ohne Abweichung
- [X] T083 Board-only-Footprint `hardware/lib/oven.pretty/Netzabdeckung_REVA.kicad_mod` (Referenz COV1, Attribute „nur Platine“ und „nicht in Stückliste“) nach research R19: Umriss beider Schalen auf F.Fab/B.Fab (einschließlich 3 mm Überstand an den Platinenkanten), Rule Areas „keine Footprints“ auf F und B entlang der Innenwand (Wand 2 mm + je 0,5 mm Toleranz), NPTH-Pads H5–H8 Ø 3,2 mm ohne Kupfer, 3D-Modell `${KIPRJMOD}/cover/netzabdeckung_oben.step` und `netzabdeckung_unten.step`; Eintrag in `hardware/lib/README.md`. In `hardware/controller/controller.kicad_dru` Regel `cover_holes_to_mains` ergänzen: „Bohrungen von COV1 ≥ 6 mm zu Netzklasse `Mains`“ (`hole_clearance` und `clearance`, Bedingung `A.Parent == 'COV1' && B.NetClass == 'Mains'`)
  - Stand 2026-10-26: Footprint erzeugt von `hardware/tools/make_cover_footprint.py` (Parameter für Wandlage, H5–H8 und Hülsen oben im Skript); Regel `cover_holes_to_mains` mit `physical_hole_clearance` 6 mm, weil `hole_clearance` NPTH-Bohrungen nicht prüft (getestet; `mains_to_mounting_holes` ebenso umgestellt). Probeplatzierung auf einer Kopie der Platine: Sperrflächen melden J2, U2, U9, D4, C15, R10, J5; H8 liegt auf dem Haltepin von J5 – Eingabe für T084. 3D-Modelle folgen mit T085
- [X] T084 COV1 in `hardware/controller/controller.kicad_pcb` platzieren (Teil der Feinplatzierung T020): Innenwand im kupferfreien Isolationsband zwischen Netz- und Kleinspannungskupfer, Abdeckung umschließt J1, F1, RV1 und das ganze IRM-10-5; H5–H8 so, dass zwei Schrauben entlang einer Wand höchstens 50 mm auseinander liegen; J5/U5 vom IRM-10-5 abrücken und J2, Q3–Q5, U7, C2, C3 nach rechts schieben, bis die Sperrflächen frei sind; liegen H1/H3 weiter im Abdeckbereich, im Footprint die Hülsen-Umrisse auf F.Fab/B.Fab ergänzen. DRC mit `--schematic-parity`: keine Verletzung durch COV1 (Courtyard/Rule Area, `cover_holes_to_mains`)
  - Erledigt 2026-10-27: COV1 im Ursprung; H8 innen unter dem IRM-10-5 bei (46,5 / 96) als Dom an der rechten Wand, weil der Flansch außen J5/J6 blockiert hätte (research R19, Ergebnis T084; Footprint neu erzeugt). Verschoben: J2, U2, U9, J5, J6; Q3–Q5, U7, C2 und C3 waren schon frei. Dabei aus T020 erledigt: C27–C30 und die Messbausteine neu um U5/U6 gruppiert (Filter links an T+/T−, Entkopplung oben, CS-Pull-up und FAULT-Diode auf der SPI-Seite); J10 an der Oberkante zwischen ESP32-Modul und USB-C (J7 um 4 mm nach rechts), Lüfterschaltung darunter. DRC `--schematic-parity`: nur offene Verbindungen und 4 Hinweise zur Beschriftung an der Kante; Gegenprobe mit J5 in der Wand meldet `items_not_allowed`
- [ ] T021 Routing, GND-Fläche unten (nur Kleinspannung), LDO-Kupferfläche ≥ 4 cm², 4 Montagelöcher M3 **ohne Kupfer**, Testpunkte TP1–TP12 laut contracts/connectors.md (keine im Netzbereich), Bestückungsdruck: „REV A", Projektname, „230 V~"-Warnung, Funktion und Pinbelegung an jedem Steckverbinder (J10: „FAN 5V“), Polarität J5/J6 in `hardware/controller/controller.kicad_pcb`
- [ ] T022 `kicad-cli pcb drc --exit-code-violations` ohne Verletzungen; Exporte nach `hardware/controller/output/`: Gerber, Bohrdaten, BOM, Bestückungsplan, STEP; Bauteilhöhe ≤ 40 mm im STEP prüfen, unter der Netzabdeckung ≤ 25 mm
- [ ] T023 [P] Encoder-Träger: Projekt, Layout (ca. 25 × 30 mm, Encoder zentriert, IDC am Rand), DRC und Exporte in `hardware/encoder-carrier/`
- [ ] T085 Netzabdeckung in Fusion 360 konstruieren nach research R19 gegen den STEP-Export aus T022: Ober- und Unterschale aus PC, Wand 2 mm, Innenhöhe oben 25 mm und unten 4 mm, Stufenfuge an den Platinenkanten, Spalte konstruktiv ≤ 0,5 mm, Bohrungen H5–H8 für Polyamid-Schrauben M3 mit Mutternaufnahme (H8 als Dom innen an der rechten Wand, research R19 Ergebnis T084), geschlossene Hülsen an H1/H3 (falls im Abdeckbereich), Kabeldurchführung als Rundloch aus zwei Halbschalen (Kabeldurchmesser aus T082 + 0,3 mm, Mantel ≥ 5 mm innen), Kabelbinder-Steg außen an der Unterschale; Umriss gegen COV1 prüfen. Ablage in `hardware/controller/cover/`: `netzabdeckung.f3d`, `netzabdeckung_oben.step/.stl`, `netzabdeckung_unten.step/.stl`, `README.md` mit Druckeinstellungen (Filament, 100 % Füllung in Wänden und Deckel, Ausrichtung); STEP-Dateien als 3D-Modell von COV1 einbinden
- [ ] T024 Sicherheits-Review der Hardware: `/speckit-checklist` gegen Prinzip IVa/IVb ausführen (einschließlich Netzabdeckung: FR-039, SC-010) (Ergebnis `specs/002-controller-pcb/checklists/safety.md`), Selbstprüfung gegen Prinzip I–X, Abweichungen beheben
- [ ] T025 Bestellung von Platinen und Bauteilen (Benutzer), einschließlich Noctua NF-A4x10 5V PWM und ggf. Verlängerung NA-SEC1, Polyamid-Schrauben M3 mit Muttern (6 Stück), PC-Filament aus T082 und Mantelleitung H05VV-F 2 × 0,75 mm²; Bezugsquellen und Preise in `docs/hardware-umbau.md` nachtragen
- [ ] T026 Bestückung (Benutzer; Drahtenden im Netzbereich auf der Unterseite auf ≤ 2 mm kürzen) und stromlose Prüfung nach `specs/002-controller-pcb/quickstart.md` Abschnitt 2; Ergebnis in `docs/compatibility.md` (Spalte Bemerkung)

### Firmware-Grundlagen (parallel zur Hardware)

- [ ] T027 [P] Fehlercodes E106 `ThermocoupleOverUnderVoltage`, E107 `ChamberOverTemperature`, E108 `ColdJunctionOutOfRange`, E201 `PcbOverTemperature`, E202 `PcbTemperatureUnknown`, E203 `EnableLatchMismatch`, E204 `UnsupportedPcbRevision`, E205 `HardwareTrip`, E206 `FanFailure` in `firmware/lib/diagnostics/include/error_code.h` und mit Bedeutung/Ursache/Abhilfe in `docs/error-codes.md`
- [ ] T028 [P] Konstanten in `firmware/include/app_config.h`: `kPcbLimitCelsius = 60`, `kPcbRecoverCelsius = 55`, `kPcbRecoverHoldMs = 5000`, `kHeartbeatPeriodMs = 250`, `kHeartbeatMaxAgeMs = 500`, `kChamberFaultLimitCelsius = 300` (Inbetriebnahme-Wert), `kLatchMismatchMs = 10`, `kStrapSampleCount = 8`, Lüfter (FR-037/FR-038): `kFanMinDutyPercent = 20`, `kFanRampStartCelsius = 35`, `kFanFullSpeedCelsius = 50`, `kFanFailRpm = 500`, `kFanFailHoldMs = 3000`, `kFanStartupGraceMs = 3000`, `kFanUpdatePeriodMs = 1000`
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
- [ ] T074 [P] [US1] Host-Tests `firmware/test/test_cooling/test_fan_curve.cpp` für `fanDutyPercent()`: „`celsius ≤ 35` → 20 %“, „`35 < celsius < 50` → linear 20…100 %“ (z. B. 42,5 °C → 60 %), „`celsius ≥ 50` → 100 %“, „Platinentemperatur ungültig (E202) → 100 %“; Ergebnis nie unter 20 %
- [ ] T035 [P] [US1] Host-Tests für zwei Temperaturen in der Statuszeile in `firmware/test/test_ui/test_status_formatter.cpp` (oben/unten, Fehlercode je Kanal z. B. „E101 oben")

### Implementierung für User Story 1

- [ ] T036 [P] [US1] `firmware/lib/temperature/include/max31856_decoder.h` + `src/max31856_decoder.cpp` – macht T032 grün
- [ ] T037 [US1] `firmware/lib/temperature/include/i_max31856_bus.h`, `max31856_reader.{h,cpp}` laut contracts/interfaces.md – macht T033 grün
- [ ] T038 [P] [US1] Modul `firmware/lib/board/` mit `i_revision_strap.h`, `pcb_revision.h` (`PcbRevisionId`), `revision_detector.{h,cpp}` – macht T034 grün
- [ ] T075 [US1] Modul `firmware/lib/cooling/` mit `include/i_fan_output.h`, `include/i_fan_tachometer.h`, `include/fan_curve.h` und `src/fan_curve.cpp` laut contracts/interfaces.md (Abschnitt cooling), Konstanten aus `app_config.h` – macht T074 grün
- [ ] T039 [US1] `firmware/lib/ui/` Statuszeile mit zwei Temperaturen (StatusFormatter, StatusBar, SystemStatus) – macht T035 grün; REV DEV zeigt weiter einen Wert
- [ ] T040 [P] [US1] HAL `firmware/lib/hal_esp32/esp32_max31856_bus.{h,cpp}`: SPI3 über GPIO-Matrix (MISO 13, SCK 14, MOSI 15), Modus 1, `kThermocoupleSpiHz`, CS je Instanz
- [ ] T041 [P] [US1] HAL `firmware/lib/hal_esp32/esp32_revision_strap.{h,cpp}`: ADC1 an GPIO 1, `kStrapSampleCount` Messungen, kalibrierte mV, Mittelwert; REV DEV liefert „kein Teiler"
- [ ] T076 [P] [US1] HAL `firmware/lib/hal_esp32/esp32_fan_output.{h,cpp}` (LEDC an `kPinFanPwm`, `kFanPwmHz` = 25 kHz, Ausgang invertiert wegen BSS138: GPIO high = PWM low; `setDutyPercent(100)` = Ausgang dauerhaft low) und `esp32_fan_tachometer.{h,cpp}` (PCNT an `kPinFanTach`, steigende Flanken, Glitch-Filter ca. 1 µs, `takePulseCount()` nicht blockierend); nur für REV A gebaut
- [ ] T042 [US1] HAL-Anpassungen für den ESP32-S3 in `firmware/lib/hal_esp32/`: Encoder an GPIO 39–41 ohne interne Pull-ups (externe vorhanden), `kEncoderStepsPerDetent` aus dem Board-Header; LovyanGFX auf SPI2 mit Pins aus `board_rev_a.h` und `kTftSpiHz`; Hintergrundbeleuchtung per LEDC an GPIO 7 (P-MOSFET: LOW = an); serielle Ausgabe über USB-CDC mit Sende-Timeout 0 ms (nie blockierend); Diagnose-LED an GPIO 2 blinkt im Betrieb
- [ ] T043 [US1] Sicherer Start in `firmware/lib/hal_esp32/esp32_heater_outputs.{h,cpp}` und `esp32_enable_latch.{h,cpp}` (Minimalumfang): GPIO 4/5/6 als Ausgang LOW vor allem anderen, DISARM-Puls an GPIO 18 als erste Aktion in `setup()`
- [ ] T044 [US1] Composition Root REV A in `firmware/src/main.cpp` (`#if OVEN_BOARD_REV_A`): sicherer Start (T043), Revisionsprüfung (E204 → Heizausgänge aus, DISARM, Fehleranzeige, kein Menü), zwei `Max31856Reader` mit `configure()`, zwei `OvenTemperatureMonitor` („oben", „unten"), Log `msg="revision"`; danach Ablauf aus Feature 001. `pio run -e esp32s3_reva` muss danach bauen; CI-Schritt „Build firmware (esp32s3_reva)" in `.github/workflows/firmware.yml` ergänzen (aus T004 verschoben)
- [ ] T077 [US1] Lüfter in die Composition Root REV A (`firmware/src/main.cpp`) einbinden: direkt nach dem sicheren Start `setDutyPercent(100)`, dann alle `kFanUpdatePeriodMs` Sollwert aus `fanDutyPercent()` mit der Platinentemperatur (Maximum der CJ-Werte), Drehzahl = Pulse/s × 60 ÷ 2, Log `lvl=INFO mod=cooling msg="fan" duty=… rpm=… pcb_c=…` (contracts/interfaces.md); REV DEV ohne Lüfter
- [ ] T078 [P] [US1] HIL-Test `firmware/test/hil/test_hil_fan/test_main.cpp` (automatischer Teil): 20 %, 50 %, 100 % je 5 s ansteuern → Drehzahl steigt monoton, bei 20 % ≥ 800 U/min (quickstart Abschnitt 4)
- [ ] T045 [P] [US1] HIL-Test `firmware/test/hil/test_hil_revision/test_main.cpp`: Teilerspannung im Fenster REV A, Erkennung = A
- [ ] T046 [P] [US1] HIL-Test `firmware/test/hil/test_hil_max31856/test_main.cpp`: Konfiguration beider Kanäle bestätigt, Kammertemperatur und CJ bei Raumtemperatur plausibel (10–40 °C), Kanäle ≤ 2 °C auseinander, abgezogener Fühler (Testanweisung im Log) → E101 ≤ 2 s
- [ ] T086 [US1] Netzabdeckung drucken (Benutzer, `hardware/controller/cover/README.md`) und nach `specs/002-controller-pcb/quickstart.md` Abschnitt 2a stromlos abnehmen: Montage mit Polyamid-Schrauben, Prüfdraht 1 mm an jeder Fuge, an der Kabeldurchführung, an den Hülsen und von unten ohne Durchgang zu L/N (SC-010), Zugtest ca. 30 N; Ergebnis in `docs/compatibility.md`. Ohne bestandene Abnahme kein Netzbetrieb
- [ ] T047 [US1] Abnahme nach `specs/002-controller-pcb/quickstart.md` Abschnitte 3 und 5 (einschließlich 5.4 Messen bei montierter Abdeckung und 5.5 Erwärmung des IRM-10-5 ≤ 10 K; sonst Labyrinth-Lamellen nach research R19 in T085 nachziehen) durchführen, Ergebnis in `docs/compatibility.md`

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

- [ ] T079 [P] [US2] Host-Tests `firmware/test/test_cooling/test_fan_monitor.cpp` mit `firmware/test/fakes/fake_fan_output.h` und `fake_fan_tachometer.h`: Sollwert aus der Kennlinie wird an `IFanOutput` gegeben; „`rpm < 500` für 3 s → E206“; kurzer Einbruch unter 3 s → kein Fehler; „die ersten 3 s nach dem Start werden nicht bewertet“; E206 bleibt gesetzt („verriegelnd bis zur Quittierung“), `acknowledge()` löscht nur, wenn wieder ≥ 500 U/min gemessen werden; Log `msg="fan failure" code=E206`

### Implementierung für User Story 2

- [ ] T051 [US2] Modul `firmware/lib/heater/` mit `heater_channel.h`, `i_heater_outputs.h`, `i_enable_latch.h`, `i_stop_button.h`, `i_external_watchdog.h` laut contracts/interfaces.md; `EnableMonitor` als neue Klasse in `contracts/interfaces.md` ergänzen (Signatur: `update(Milliseconds now, bool firmwareArmed, bool firmwareDisarmed)`, `ErrorCode status()`, `const char* tripCause()`)
- [ ] T052 [US2] `firmware/lib/heater/src/safety_heartbeat.cpp` – macht T048 grün
- [ ] T053 [US2] `firmware/lib/heater/src/enable_monitor.cpp` – macht T050 grün
- [ ] T054 [US2] `firmware/lib/board/src/pcb_temperature_monitor.cpp` – macht T049 grün
- [ ] T080 [US2] `firmware/lib/cooling/include/fan_monitor.h` und `src/fan_monitor.cpp` laut contracts/interfaces.md – macht T079 grün
- [ ] T055 [P] [US2] HAL vervollständigen: `firmware/lib/hal_esp32/esp32_enable_latch.{h,cpp}` (ARM: steigende Flanke an GPIO 6; DISARM: LOW-Puls 100 µs an GPIO 18 (Datenblatt: ≥ 1 µs); `isArmed()` GPIO 47; `isClearActive()` GPIO 36 LOW), `esp32_stop_button.{h,cpp}` (GPIO 42, LOW = gedrückt, entprellt), `esp32_external_watchdog.{h,cpp}` (GPIO 21 umschalten)
- [ ] T056 [US2] Integration in `firmware/lib/app/` und `firmware/src/main.cpp`: Scheduler-Aufgaben Herzschlag (jeder Durchlauf, `SafetyHeartbeat::tick`), Platinentemperatur 250 ms, Freigabeüberwachung 10 ms; jede Meldung E1xx/E201–E206 → `allOff()`, `disarm()`, Log und Fehleranzeige mit Code; Normalbetrieb ruft `arm()` nie auf. `FanMonitor` ersetzt die Lüfter-Aufgabe aus T077 (Periode `kFanUpdatePeriodMs`); E206 wird auch ohne Heizvorgang angezeigt (FR-037)
- [ ] T057 [US2] HIL-Test `firmware/test/hil/test_hil_enable_chain/test_main.cpp`: Q nach Start gelöscht; ARM setzt Q; DISARM löscht ≤ 1 ms; Herzschlag stoppen → Q gelöscht ≤ Watchdog-Maximalzeit aus T006; Kammer-Obergrenze unter Raumtemperatur setzen → Q gelöscht und `clearActive`; ARM bei aktivem Stopp wirkungslos; manuelle Schritte M1–M4 mit Testanweisung im Log und Prüfung von Q (quickstart Abschnitt 4)
- [ ] T081 [US2] HIL-Test `firmware/test/hil/test_hil_fan/test_main.cpp` um die manuellen Schritte M5–M7 erweitern (Testanweisung im Log): Freigabe setzen, Lüfter festhalten bzw. J10 abziehen → E206 ≤ 5 s und Q gelöscht (SC-009, US2-9); ohne Freigabe Lüfter abziehen → E206 angezeigt; Platine mit Heißluft auf ca. 40/50 °C → Drehzahl steigt, ab 50 °C volle Drehzahl; Ergebnis in `docs/compatibility.md`
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

- [ ] T063 [P] [US4] Übergabe an das Gehäuse-Feature: Maßzeichnung (Außenmaße, Montagelöcher, Steckverbinderlage, höchstes Bauteil) Bauraum der Netzabdeckung (ca. 27 mm über und 6 mm unter der Platine, 3 mm Überstand links und unten, STEP aus `hardware/controller/cover/`) und Vorgaben für die aktive Lüftung (40-mm-Lüfter Noctua NF-A4x10 in der Wand des Bedienschachts, bläst Außenluft über die Platine; Lage von J10, Kabellänge 20 cm + 30 cm Verlängerung; Lüfterausschnitt, Gitter und Abluftöffnungen legt das Gehäuse-Feature fest, Prinzip X) als Abschnitt „Übergabe Platine REV A" in `docs/hardware-umbau.md`, STEP aus `hardware/controller/output/` und `hardware/encoder-carrier/output/`
- [ ] T064 [P] [US4] Montage von Antennen-Pigtail (RP-SMA-Durchführung) und Panel-USB-Verlängerung in `docs/hardware-umbau.md` beschreiben (Lochdurchmesser, Lage, Kabellängen)
- [ ] T065 [US4] quickstart Abschnitt 7 durchführen (WLAN bei geschlossenem Gehäuse, Upload über die Wandbuchse, Aus- und Einbau < 10 min); Ergebnis in `docs/compatibility.md`

**Checkpoint**: SC-007 und SC-008 belegt.

---

## Phase 7: Polish & Cross-Cutting Concerns

- [ ] T066 [P] `docs/compatibility.md`: Zeile Firmware 0.2.0 ↔ PCB REV A ↔ Gehäuse „–" mit HIL- und Abnahmeergebnis; Zeile 0.2.0 ↔ REV DEV (weiterhin unterstützt)
- [ ] T067 [P] `firmware/include/board/board_rev_a.h`: Pins gegen Datenblätter (ESP32-S3, MAX31856, TPS3828, SN74LVC74A, Noctua-PWM-Whitepaper) und KiCad-Netzliste geprüft, Kommentar „pins verified against datasheet: yes (<Datum>)"
- [ ] T068 [P] `docs/hardware-umbau.md`: Komponententabelle mit endgültigen Bauteilen, Bezugsquellen, Preisen; Status „REV A gefertigt"
- [ ] T069 `pio test -e native` (inkl. `test_cooling`), `pio run -e esp32_revdev`, `pio run -e esp32s3_reva`, `pio check`, clang-format; ohne neue Warnungen (Qualitätstore der Verfassung)
- [ ] T070 Abschluss-Review: Selbstprüfung gegen Prinzip I–X, `/code-review` für die Firmware-Änderungen, `/speckit-analyze` für Spec/Plan/Tasks; Abweichungen beheben oder begründen

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: keine; T003 (Spike) vor T009–T018.
- **Foundational (Phase 2)**: T006–T009 vor dem Schaltplan; T010–T016 parallel; T017 nach
  T010–T016; T019–T022 nacheinander nach T017; T024 vor T025; T026 nach Lieferung.
  Firmware-Grundlagen T027–T031 parallel zur Hardware. Lüfter: T072 vor T073; T073 nach T071
  (gleiche Schaltplandatei, KiCad geschlossen) und vor der Feinplatzierung (T020) und T021.
  Netzabdeckung: T082 und T083 vor T084; T084 vor T021; T085 nach T022 und vor T024/T025.
- **US1 (Phase 3)**: Tests und Implementierung ab Ende der Firmware-Grundlagen; HIL und Abnahme
  (T045–T047, T078) erst nach T026; T086 vor jedem Netzbetrieb (quickstart 5) und damit vor T047. Lüfter: T074 → T075 → T077; T076 parallel; T077 nach T044.
- **US2 (Phase 4)**: nach US1 (nutzt Start, Messkanäle und Board-Modul); T058 braucht das Lastteil.
  Lüfter: T079 → T080 → T056; T081 nach T056 und T078.
- **US3 (Phase 5)**: nach US2 (Heizen nur mit funktionierender Freigabe).
- **US4 (Phase 6)**: T063/T064 nach T022; T065 nach US1.
- **Polish**: nach den gewünschten Stories.

### Within Each User Story

- Tests zuerst, müssen fehlschlagen; dann Logik, HAL, Integration, HIL, Abnahme.

### Parallel Opportunities

- Phase 1: T004 und T005 parallel zu T001–T003.
- Phase 2: T007/T008 parallel; T072 und T082 parallel; Schaltplanblöcke T010–T016 parallel; T018 und T023 parallel zur
  Hauptplatine; T027–T031 parallel zur gesamten Hardware.
- US1: T032–T035 und T074 parallel; T036, T038, T040, T041, T076 parallel; T045/T046/T078 parallel.
- US2: T048–T050 und T079 parallel; T055 parallel zu T052–T054 und T080.

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
