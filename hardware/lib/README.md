# Bauteilbibliothek und Prüfliste

Projekteigene Symbole (`oven.kicad_sym`) und Footprints (`oven.pretty/`) für Teile, die in
KiCad 10 fehlen. Alle anderen Teile kommen aus den Standardbibliotheken von KiCad 10.0.3.

Regel: Ein Teil darf erst in den Schaltplan, wenn seine Pinbelegung und sein Footprint gegen das
Datenblatt geprüft und hier eingetragen sind.

## Projekteigene Teile

| Teil | Datei | Inhalt |
|---|---|---|
| TC6501 | `oven.kicad_sym` | Symbol; Footprint `Package_TO_SOT_SMD:SOT-23-5` (SOT-23A = SC-74A) |
| TPS3828-33DBV | `oven.kicad_sym` | Symbol wie TPS3823-xxDBV aus KiCad, aber ~{RESET} als Open-Drain (ERC an der Löschleitung) |
| Omega PCC-SMP | `oven.pretty/Omega_PCC-SMP_Horizontal.kicad_mod` | Footprint der Thermoelement-Printbuchse, **ungeprüft** (siehe unten) |

## Prüfliste

| Bauteil | Symbol / Footprint | Quelle (Datenblatt) | Ergebnis | Geprüft am |
|---|---|---|---|---|
| ESP32-S3-WROOM-1U-N8 | `RF_Module:ESP32-S3-WROOM-1` / `RF_Module:ESP32-S3-WROOM-1U` | Espressif ESP32-S3-WROOM-1/1U Datasheet v1.8, Tab. 3-1 | alle 41 Pins stimmen | 2026-10-25 |
| MAX31856 | `Sensor_Temperature:MAX31856` / `Package_SO:TSSOP-14_4.4x5mm_P0.65mm` | KiCad-Symbol und zweite Quelle (Pin-Tabelle LCSC/Flux) | 14 Pins stimmen; Originaldatenblatt war nicht abrufbar. Ausgangstyp FAULT unklar → Anbindung über Schottky-Diode (research R6) | 2026-10-25 |
| TPS3828-33 | `oven:TPS3828-33DBV` / `Package_TO_SOT_SMD:SOT-23-5` | TI SLVS165O (März 2025), Pin-Tabelle DBV | 1 ~RESET, 2 GND, 3 ~MR, 4 WDI, 5 VDD | 2026-10-25 |
| TC6501P065 | `oven:TC6501` / `Package_TO_SOT_SMD:SOT-23-5` | Microchip DS21451E, „Package Types" | 1 GND, 2 GND, 3 HYST, 4 VCC, 5 ~TOVER (Open-Drain) | 2026-10-25 |
| SN74LVC74A | `74xx:74HC74` (pinkompatibel) / `Package_SO:SOIC-14_3.9x8.7mm_P1.27mm` | TI SN74LVC74A, Tab. 4-1 | 1 1CLR, 2 1D, 3 1CLK, 4 1PRE, 5 1Q, 6 1Q̄, 7 GND, 8 2Q̄, 9 2Q, 10 2PRE, 11 2CLK, 12 2D, 13 2CLR, 14 VCC | 2026-10-25 |
| AP7361C-33E | `Regulator_Linear:AP7361C-33E` / `Package_TO_SOT_SMD:SOT-223-3_TabPin2` | Diodes AP7361C, Pin Descriptions (SOT223) | 1 IN, 2 GND (Tab), 3 OUT; Ausgang ≥ 2,2 µF Keramik | 2026-10-25 |
| IRM-10-5 | `Converter_ACDC:IRM-10-5` / `Converter_ACDC:Converter_ACDC_MeanWell_IRM-10-xx_THT` | Mean Well IRM-10-SPEC 2025-08-08, Maßzeichnung (Ansicht von unten) | Pad 1 AC/N, 2 AC/L (Abstand 10,75), 3 −V, 4 +V (Abstand 8, 38,5 vom Eingang): stimmt mit Zeichnung überein. Maße 45,7 × 25,4 × 21,5 mm, −30…+85 °C | 2026-10-25 |
| USBLC6-2SC6 | `Power_Protection:USBLC6-2SC6` / `Package_TO_SOT_SMD:SOT-23-6` | KiCad-Symbol und ST-Funktionsbild (DS4260, Sekundärquelle) | 1/6 I/O1, 2 GND, 3/4 I/O2, 5 VBUS | 2026-10-25 |
| USB-C GCT USB4085-GF-A | `Connector:USB_C_Receptacle_USB2.0_16P` / `Connector_USB:USB_C_Receptacle_GCT_USB4085` | Footprint aus KiCad, Pads A1–A12, B1–B12, SH | Padnamen passen zum Symbol; Herstellerzeichnung beim Bestellen prüfen | 2026-10-25 |
| Omega PCC-SMP-K | `Connector_Generic:Conn_01x02` (1 = +, 2 = −) / `oven:Omega_PCC-SMP_Horizontal` | Omega Katalogblatt G-38 (Maße nur isometrisch) | **Ungeprüft**: quer 7,9 mm (Kontakte) und 15,7 mm (Halter), Lochdurchmesser 1,77 mm sind eindeutig; der Abstand Kontakte–Halter (13,5 mm) ist aus der Isometrie gelesen. **Vor dem Layout an einem Musterteil nachmessen.** Polarität: von oben gesehen, Steckseite zu +Y, liegt + rechts | 2026-10-25 |
| MOSFETs IRLML6346, BSS138, AO3401A | `Device:Q_NMOS_GSD` / `Q_PMOS_GSD`, `Package_TO_SOT_SMD:SOT-23` | Standard-SOT-23: 1 G, 2 S, 3 D | beim Platzieren je Typ gegen das Datenblatt prüfen | offen |
| SS34, MBR0530 | `Device:D_Schottky` / `Diode_SMD:D_SMA`, `D_SOD-123` | Kathodenring | beim Platzieren prüfen | offen |
