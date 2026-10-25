# Quickstart: Controller-Platine REV A – Bring-up, HIL und Abnahme

Prüfablauf, der belegt, dass die Platine die Spec erfüllt. Belegungen:
[contracts/connectors.md](contracts/connectors.md); Firmware-Schnittstellen:
[contracts/interfaces.md](contracts/interfaces.md).

> ⚠ Schritte mit Netzspannung nur mit Trenntrafo oder FI-geschützter Steckdose, Platine nie im
> eingeschalteten Zustand berühren. Bis Schritt 5 wird **ohne** Netz gearbeitet.

## Voraussetzungen

- KiCad 10.0.3 unter `D:\hobby\Elektronik\KiCad\10.0` (`kicad-cli` in `bin`)
- Python 3.13, `hardware/.venv` mit `pip install -r hardware/requirements.txt`
- PlatformIO
- Bestückte Platine REV A, Encoder-Träger, Display, zwei Thermoelemente Typ K, Stopp-Taste,
  USB-C-Kabel; für Schritt 6 Lastteil mit **Ersatzlast** (z. B. zwei Glühlampen 40 W) statt
  Heizstäben; Multimeter, Referenzthermometer

## 1. Hardware-Daten erzeugen und prüfen

```bash
hardware/.venv/Scripts/python hardware/controller/schematic/main.py
```

```bash
"D:/hobby/Elektronik/KiCad/10.0/bin/kicad-cli.exe" pcb drc --exit-code-violations hardware/controller/controller.kicad_pcb
```

Erwartet: SKiDL-ERC ohne Fehler, Netzliste erzeugt; DRC ohne Verletzungen (inkl.
Netzklassen-Abstände 6 mm primär/sekundär). Danach Exporte (Gerber, Bohrdaten, BOM, STEP) laut
`hardware/README.md`.

## 2. Sichtprüfung und Widerstandsmessung (stromlos)

- „REV A" im Bestückungsdruck, keine Testpunkte im Netzbereich, Frässchlitz vorhanden.
- Kein Durchgang zwischen GND und Montagelöchern.
- Kein Kurzschluss 5 V–GND, 3,3 V–GND, L–N.

## 3. Erster Start über USB (ohne Netz)

```bash
pio run -d firmware -e esp32s3_reva -t upload
```

| # | Aktion | Erwartet | Spec |
|---|---|---|---|
| 3.1 | USB anstecken | 5-V-Schiene ca. 4,5 V, 3,3 V ± 3 %; Diagnose-LED blinkt | FR-003 |
| 3.2 | Firmware über USB aufspielen (ohne Adapter) | Upload erfolgreich | US1-1, FR-004 |
| 3.3 | Start beobachten | Begrüßung mit Version 0.2.0 und „REV A"; Log `msg="revision" detected=A` | US1-2, FR-007 |
| 3.4 | Encoder drehen/drücken, Stopp drücken | Menü reagiert; Log zeigt Stopp | US1-4 |
| 3.5 | Fühler oben/unten bei Raumtemperatur | beide ≤ 3 °C vom Referenzthermometer, ≤ 2 °C voneinander | US1-3, SC-005 |
| 3.6 | Fühler abziehen | „E101 oben" bzw. „unten" innerhalb 2 s | SC-006 |

## 4. Host- und HIL-Tests

```bash
pio test -d firmware -e native
```

```bash
pio test -d firmware -e esp32s3_reva
```

Automatische HIL-Tests (ohne Netz, Last nicht nötig):

| Test | Prüft | Spec |
|---|---|---|
| `test_hil_revision` | Teilerspannung im Fenster REV A | FR-007 |
| `test_hil_max31856` | Konfiguration, beide Kanäle lesen, CJ plausibel | FR-013, FR-015 |
| `test_hil_enable_chain` · Start | Q nach Start gelöscht | US2-1, FR-020 |
| · ARM/DISARM | ARM setzt Q, DISARM löscht in ≤ 1 ms | FR-022, FR-023 |
| · Watchdog | Herzschlag stoppen → Q gelöscht in ≤ 2 s | US2-4, SC-002 |
| · Kammer-FAULT | Obergrenze per Register unter Raumtemperatur setzen → Q gelöscht, `clearActive` | US2-6, FR-014 |
| · Vorrang Löschen | Stopp gedrückt halten (Testanweisung im Log) → ARM wirkungslos | US2-7 |

Manuelle HIL-Schritte (Test fordert per Log dazu auf und prüft das Ergebnis):

| Schritt | Aktion | Erwartet | Spec |
|---|---|---|---|
| M1 | Freigabe setzen, Stopp drücken und loslassen | Q gelöscht und bleibt gelöscht | US2-2 |
| M2 | Freigabe setzen, Bedienteil-Kabel abziehen | Q gelöscht | US2-3 |
| M3 | Freigabe setzen, TC6501 mit Heißluft (< 100 °C) erwärmen | Q gelöscht bei ca. 65 °C, bleibt nach Abkühlen gelöscht | US2-5 |
| M4 | Freigabe setzen, 3,3 V über Labornetzteil absenken (Testaufbau) oder Brownout simulieren | Q gelöscht | FR-021 |

Ergebnis in `docs/compatibility.md` eintragen (Prinzip III).

## 5. Netzbetrieb (ohne Last)

| # | Aktion | Erwartet | Spec |
|---|---|---|---|
| 5.1 | Netz einschalten, USB getrennt | Start wie 3.3 | FR-001 |
| 5.2 | USB während Netzbetrieb anstecken und wieder trennen | kein Neustart; am USB-Kabel keine Rückspeisung (Messung VBUS bei getrenntem PC-Ende: 0 V) | US1-5, FR-003 |
| 5.3 | WLAN-Upload 30 s lang | keine Neustarts, 3,3 V ≥ 3,2 V | FR-001 |

## 6. Lastteil mit Ersatzlast

| # | Aktion | Erwartet | Spec |
|---|---|---|---|
| 6.1 | HIL-Testmodus: Freigabe setzen, nur oben ein | nur Lampe oben leuchtet | US3-1 |
| 6.2 | Beide im Sekundentakt, 60 s | beide folgen ohne Aussetzer | US3-2 |
| 6.3 | Freigabe gelöscht, Heizbefehl an | keine Lampe | US3-3, FR-019 |
| 6.4 | Lampe an, Stopp drücken, Zeit bis „aus" messen (Oszilloskop oder Logikanalysator am SSR-Eingang + Relais) | ≤ 100 ms | SC-001 |
| 6.5 | SSR-Kurzschluss simulieren (SSR-Ausgang mit isolierter Brücke überbrücken), Freigabe löschen | Relais trennt, Lampe aus | US2-8 |
| 6.6 | 100 × Einschalten und 100 × Neustart (Skript per USB-Reset) mit Logikanalysator an SSR-Eingängen | kein Einschaltimpuls | SC-004 |

## 7. Einbau

| # | Aktion | Erwartet | Spec |
|---|---|---|---|
| 7.1 | Einbau im Bedienschacht, Gehäuse zu | WLAN verbunden | US4-1, SC-007 |
| 7.2 | USB an der Gehäusewand, Firmware aufspielen | erfolgreich | US4-2, SC-007 |
| 7.3 | Aus- und Einbau mit Uhr | < 10 min, ohne Löten | US4-3, SC-008 |
