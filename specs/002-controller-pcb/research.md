# Research: Controller-Platine (PCB REV A)

Grundlage: [spec.md](spec.md) und die Entscheidungen in `docs/hardware-umbau.md` (Stand
2026-10-25). Hier stehen die Festlegungen für Schaltung, Layout und Firmware mit Begründung.
Werte mit „prüfen" werden beim Anlegen des jeweiligen Symbols bzw. Footprints gegen das
Datenblatt bestätigt (Regel aus `docs/werkzeuge-pcb.md`).

## R1 – Werkzeugkette: SKiDL + KiCad 10

- **Decision**: Schaltplan als Python-Code mit SKiDL, Ausgabe als KiCad-Netzliste; Layout,
  ERC/DRC und Exporte in KiCad 10.0.3 (`D:\hobby\Elektronik\KiCad\10.0`, `kicad-cli`).
  SKiDL läuft in einer eigenen virtuellen Umgebung `hardware/.venv` (Python 3.13), Version in
  `hardware/requirements.txt` fixiert.
- **Rationale**: Netzlisten sind zwischen KiCad-Versionen stabil; kein fremdes Werkzeug schreibt
  in die `.kicad_pcb`. Python-Code ist reviewbar und lässt sich blockweise aufbauen.
- **Risiko**: SKiDL nennt als neueste Werkzeugkennung KiCad 8/9; ob es die Symbolbibliotheken
  von KiCad 10 fehlerfrei liest, ist nicht belegt. Gegenmaßnahme: Erster Task ist ein Spike
  (ein Symbol aus KiCad 10, eine Netzliste, Import in KiCad 10). Scheitert das Lesen der
  Standardbibliotheken, kommen alle verwendeten Symbole in die projekteigene Bibliothek
  `hardware/lib/oven.kicad_sym`.
- **Ergebnis Spike (T003, 2026-10-25)**: Risiko ausgeräumt.
  - SKiDL **3.0.0** bringt eine eigene Werkzeugkennung **`KICAD10`** mit
    (`set_default_tool(KICAD10)`).
  - Mit `KICAD10_SYMBOL_DIR` = `D:\hobby\Elektronik\KiCad\10.0\share\kicad\symbols` und
    `KICAD10_FOOTPRINT_DIR` = `…\share\kicad\footprints` liest SKiDL die
    Standardbibliotheken von KiCad 10 fehlerfrei (getestet: `RF_Module:ESP32-S3-WROOM-1` mit
    41 Pins, `Sensor_Temperature:MAX31856`, `Device:R`). Das Symbol `ESP32-S3-WROOM-1` passt
    auch für das 1U-Modul; Footprint `RF_Module:ESP32-S3-WROOM-1U` (62 Pads).
  - Die erzeugte Netzliste (Format „D") lässt sich mit dem `pcbnew`-Modul aus KiCad 10 in
    eine Platine übernehmen: alle Footprints gefunden, alle Pins einem Pad zugeordnet;
    `kicad-cli pcb drc` liest die Platine ohne Footprint-Fehler.
  - Folge: Standardbibliotheken werden genutzt; `hardware/lib` nur für fehlende Teile.
    Hinweis: SKiDL warnt beim Import über fehlende Umgebungsvariablen älterer KiCad-Versionen
    und über fehlende Tags; die Schaltplanquellen setzen deshalb feste Tags je Bauteil.
- **Alternatives considered**: atopile (nur mit KiCad 9 erprobt, eigener `.kicad_pcb`-Parser
  verwirft unbekannte Inhalte); Schaltplan direkt in der KiCad-GUI (mehr Handarbeit, schlechter
  reviewbar); MCP-Server (nur Inspektion, kein Export in KiCad 10).

## R2 – Netzeingang, Netzteil und Isolation

- **Decision**:
  - Mean Well **IRM-10-5** (5 V / 2 A, Printmodul).
  - Netzeingang: steckbare Schraubklemme 2-polig, Raster 5,08 mm → Feinsicherung **T 500 mA**
    (TR5 oder Halter 5 × 20, prüfen) → Varistor **S07K275** zwischen L und N → IRM-10-5.
  - **Primär ↔ sekundär: ≥ 6,0 mm Kriech- und Luftstrecke**, dazu ein **Frässchlitz** (≥ 2 mm
    breit) unter dem Modul zwischen Netz- und Kleinspannungsseite. L ↔ N hinter der Sicherung
    ≥ 3,0 mm.
  - Netzbereich ohne Testpunkte, ohne Bestückungs-Vias, abgegrenzt im Bestückungsdruck
    („230 V") und als eigene Netzklasse mit DRC-Abstandsregel.
- **Rationale**: Verstärkte Isolierung bei 230 V, Überspannungskategorie II,
  Verschmutzungsgrad 2 verlangt nach IEC 60664-1/62368-1 rund 5 mm Kriech- und 3 mm Luftstrecke;
  6 mm mit Schlitz geben Reserve für Fertigungstoleranzen und Staub (Prinzip IVb). Zertifiziertes
  Modul statt diskretem Netzteil.
- **Temperatur**: IRM-10 je nach Datenblattstand bis 70 °C bzw. 85 °C Umgebung mit
  Leistungsreduzierung; ausgelegt wird auf 70 °C. Last ca. 0,45 A von 2 A (≈ 23 %).
- **Alternatives considered**: HLK-5M05 (Zertifikate fraglich, 5 W wenig Reserve); TNY289 aus
  dem Magazin (diskretes Netzteil, unverhältnismäßiger Isolationsaufwand).

## R3 – Controller, USB und Antenne

- **Decision**:
  - **ESP32-S3-WROOM-1U-N8** (ohne PSRAM, 85 °C), U.FL-Anschluss an der Platinenkante; Pigtail
    U.FL → RP-SMA-Durchführung, 2,4-GHz-Antenne außen.
  - USB-C-Buchse (16-polig, Durchsteck-Haltelaschen, prüfen auf Handlötbarkeit), CC1/CC2 je
    5,1 kΩ, **USBLC6-2SC6** an D+/D−/VBUS, D−/D+ an GPIO 19/20. Schirm per Bestückungsoption
    (0 Ω bzw. 1 MΩ ∥ 4,7 nF) an GND.
  - Taster BOOT (GPIO 0) und RESET (EN), EN mit RC (10 kΩ / 1 µF) nach Espressif-Empfehlung.
  - Entkopplung am Modul: 10 µF + 100 nF, zusätzlich 100 µF Elko an 3,3 V.
  - Firmware: Umgebung `esp32s3_reva` mit `board = esp32-s3-devkitc-1`, 8 MB Flash,
    Partitionstabelle mit zwei OTA-Partitionen (`default_8MB.csv`), `ARDUINO_USB_MODE=1`,
    `ARDUINO_USB_CDC_ON_BOOT=1`. Die serielle Ausgabe über USB-CDC bekommt einen
    Sende-Timeout von 0 ms, damit Logging ohne angeschlossenen Rechner nie blockiert
    (Prinzip IX/V).
- **Rationale**: Natives USB spart USB-UART-Chip und Auto-Reset; externe Antenne, weil das
  Blechgehäuse eine PCB-Antenne abschirmt; 85-°C-Modul passt zu FR-030.
- **Alternatives considered**: WROOM-1U-N16R8 aus dem Magazin (nur 65 °C, belegt GPIO 35–37);
  WROOM-1 mit PCB-Antenne (abgeschirmt im Gehäuse).

## R4 – Thermoelement-Kanäle

- **Decision**:
  - Je Kanal (oben, unten) ein **MAX31856** (TSSOP-14) an einem gemeinsamen SPI-Bus (SPI3,
    Modus 1, 1 MHz), eigene CS-Leitung mit 10-kΩ-Pull-up.
  - Konfiguration durch die Firmware nach dem Start: Thermoelement Typ K, 50-Hz-Filter,
    Automatik-Wandlung, Fühlerbrucherkennung an, Fehlermodus **Interrupt** (FAULT bleibt aktiv,
    bis die Firmware quittiert), FAULT-Maske: Kammer-Obergrenze, Fühlerbruch, Über-/Unterspannung
    und Vergleichsstelle außerhalb des Bereichs lösen FAULT aus.
  - Kammer-Obergrenze (Register): in diesem Feature ein Inbetriebnahme-Wert aus `app_config.h`
    (`kChamberFaultLimitCelsius = 300`); den endgültigen Wert legt „Sichere
    Temperaturregelung" fest.
  - FAULT (Open-Drain) beider Bausteine auf die CLR-Leitung des Freigabe-Flipflops.
  - Eingang: Mini-Thermoelement-Buchse Typ K (Printversion, Kontakte aus Thermoelement-Material)
    direkt neben dem IC, Eingangsfilter nach Datenblatt (Differenzkondensator, ggf. Ferrite).
    Buchsen und ICs auf einer gemeinsamen Kupferfläche, ohne Wärmequellen in der Nähe.
  - Die Firmware liest Register zyklisch zurück und vergleicht sie mit der Soll-Konfiguration;
    Abweichung → Fehler E105 und DISARM.
- **Rationale**: 50-Hz-Filter gegen Einstreuung aus den getakteten 230-V-Leitungen; FAULT als
  Hardware-Abschaltung bei Kammer-Übertemperatur und Fühlerfehlern (FR-014). Interrupt-Modus,
  damit ein kurzer Fehler nicht unbemerkt verschwindet.
- **Alternatives considered**: MAX31855 (kein Netzfilter, kein FAULT-Ausgang); ADS1118 mit
  eigener Vergleichsstellen-Kompensation (mehr Firmware, keine Hardware-Abschaltung).

## R5 – Platinentemperatur über die Vergleichsstelle

- **Decision**: Die Vergleichsstellen-Temperaturen (CJ) beider MAX31856 sind die
  Platinentemperatur (FR-015). Die Firmware wertet den höheren der beiden gültigen Werte aus;
  ≥ **60 °C** → E201, DISARM. Ist keiner der beiden Werte gültig → E202, behandelt wie
  Übertemperatur. Unabhängig davon schaltet der **TC6501P065** (65 °C, Hysterese 10 °C) in
  Hardware ab.
- **Rationale für Prinzip IVa („thermisch kritischste Stelle")**: SSRs und Relais sitzen nicht
  auf der Platine; die einzigen Wärmequellen sind Netzteil (ca. 0,5 W) und LDO (ca. 0,45 W).
  Die kritische Größe ist die Umgebungstemperatur im Elektronikfach. Die MAX31856 sitzen an der
  Platinenseite zur Heizkammer und messen genau diese. Den lokalen Hotspot zwischen Netzteil
  und LDO überwacht der TC6501.
- **Alternatives considered**: eigener I²C-Sensor (zusätzlicher Baustein ohne Mehrwert für die
  Fragestellung); NTC + LM358 als Hardware-Schalter (mehr Bauteile und Toleranzen).

## R6 – Freigabe-Flipflop und Löschleitung

- **Decision**:
  - **SN74LVC74A** (SOIC-14, ein Flipflop genutzt, das zweite mit festen Pegeln an den
    Eingängen) an 3,3 V. D = High, CLK = **ARM** (GPIO 6, steigende Flanke), PRE = High,
    CLR = Löschleitung.
  - **Löschleitung (aktiv low)**: 100-kΩ-Pull-down nach GND; 3,3 V nur über den
    **Stopp-Öffner** und 1 kΩ. Open-Drain-Quellen auf der Leitung: TPS3828 /RESET, TC6501,
    FAULT beider MAX31856. 1 nF gegen GND gegen Störspitzen aus dem Kabel; ESD-Schutz am
    Steckverbinder (R9).
  - Rücklesen: **Q** an GPIO 47, **Löschleitung** über 10 kΩ an GPIO 36 (`CLR_SENSE`, neu),
    Stopp-Schließer an GPIO 42.
  - Q steuert: P-MOSFET der SSR-Versorgung (über BSS138) und die Relaisspule (IRLML6346).
- **Rationale**: Asynchrones Löschen hat beim 74LVC74 Vorrang vor Takt und Setzen (FR-021);
  Power-on: TPS3828 hält /RESET 200 ms low, Q startet gelöscht (FR-020). SOIC-14 ist gut von
  Hand lötbar (FR-033). `CLR_SENSE` zeigt der Firmware, ob gerade eine Löschquelle aktiv ist
  (Akzeptanz-Szenario US2-7, Fehlermeldung statt stiller Verweigerung).
- **Alternatives considered**: 74LVC1G74 (SM8, 0,5–0,65 mm Raster, schwerer lötbar);
  diskreter Transistor-Latch (mehr Bauteile, schwer prüfbar).

## R7 – Externer Watchdog und Spannungsüberwachung

- **Decision**: **TPS3828-33** (SOT-23-5, Schwelle 2,93 V, Open-Drain /RESET, 200 ms
  Reset-Puls). WDI an GPIO 21, /MR an GPIO 18 (DISARM). /RESET nur auf die Löschleitung, nicht
  an EN des ESP32. Die Firmware schaltet WDI alle 250 ms um, **nur aus der Hauptschleife und nur,
  wenn im letzten Zyklus Temperaturüberwachung und Platinenüberwachung rechtzeitig gelaufen
  sind** (`SafetyHeartbeat`, host-testbar).
- **Prüfen beim Symbol**: (1) Watchdog-Zeit min/typ/max laut Datenblatt; liegt der Maximalwert
  über 2 s, wird SC-002 auf diesen Wert + 0,1 s angepasst oder die Variante gewechselt.
  (2) Ob ein hochohmiger WDI den Watchdog abschaltet (TI SLVA145); dann Pull-down an WDI
  bestücken, sonst Footprint unbestückt lassen.
- **Rationale**: Deckt hängende Firmware, Power-on und Unterspannung der Logik in einem
  Baustein ab; Open-Drain passt auf die gemeinsame Löschleitung.
- **Alternatives considered**: ICL7665/NE555 aus dem Magazin (kein Watchdog bzw. mehr
  Bauteile); nur interner Watchdog (deckt „GPIO bleibt high bei hängender Firmware" nicht ab).

## R8 – Lastansteuerung, SSR und Sicherheitsrelais

- **Decision**:
  - SSR-Versorgung: **AO3401A** (P-Kanal) auf der High-Side von +5 V, Gate über BSS138 vom
    Latch-Ausgang Q nach GND gezogen (Vgs ≈ −5 V), 100-kΩ-Pull-up Gate–Source.
  - Je Kanal **IRLML6346** (N-Kanal) auf der Low-Side, Gate von GPIO 4 (oben) bzw. 5 (unten) über
    100 Ω, 100 kΩ Pull-down.
  - Relaisspule (Finder 40.61, 5 V, ca. 130 mA): IRLML6346 vom Latch-Ausgang Q, Freilaufdiode
    MBR0530.
  - Lastteil-Stecker 6-polig, steckbare Schraubklemme Raster 3,81 mm (Belegung:
    [contracts/connectors.md](contracts/connectors.md)).
- **Reserve und Kühlung (Prinzip IVb)**:
  - Last je Kreis ca. 600 W / 2,6 A, gesamt ca. 5,2 A (beim Öffnen nachmessen).
  - SSR 25 A nullspannungsschaltend: Reserve ca. Faktor 10 für Einschaltstrom und Alterung der
    Heizstäbe. Verlust ca. 1,3 V × 2,6 A ≈ 3,5 W je SSR → Kühlkörper oder Montageblech mit
    Wärmeleitpad; Wärmewiderstand Kühlkörper ≤ 5 K/W hält das SSR bei 50 °C Umgebung unter
    ca. 70 °C Gehäusetemperatur.
  - Relais 16 A (AC-1) bei 5,2 A ohmscher Last: Reserve ca. Faktor 3; schaltet im Normalbetrieb
    stromlos (Firmware schaltet erst die SSRs aus).
- **Rationale**: Physische UND-Verknüpfung von Freigabe und Heizbefehl (FR-019); Bauteile aus
  dem Magazin; alle Heizpfade mit Pull-down bzw. Pull-up, damit im Reset alles aus ist.

## R9 – Bedienteil-Schnittstelle und Encoder-Träger

- **Decision**:
  - Hauptplatine: Stecker **IDC 2×4** (Raster 2,54 mm) für Encoder A/B/Taster, Stopp-Öffner
    (Hin- und Rückleitung) und Stopp-Schließer, GND.
  - Eingänge mit 10 kΩ Pull-up, 10 kΩ Serie, 10 nF nach GND (Datenblatt Alps EC11E); ESD-Schutz
    der vier Signalleitungen mit einem **USBLC6-2SC6** als vierfaches Diodenarray (VBUS-Pin an
    3,3 V), ein zweites USBLC6-2SC6 schützt die beiden Stopp-Öffner-Leitungen.
  - **Encoder-Träger** (eigene kleine Platine, ca. 25 × 30 mm): Alps EC11E15244G1, IDC 2×4 zur
    Hauptplatine, Steckklemme bzw. JST-XH für die Litzen der Stopp-Taste. Befestigung nur über die
    Mutter des Encoders.
  - Schritte pro Rastung: 2 (15 Impulse / 30 Rastungen) in `board_rev_a.h`.
- **Rationale**: ESD-Schutz mit vorhandenem Baustein; Entprellung in Hardware entlastet die
  Interrupts; der Träger hält den Encoder mechanisch sauber in der Front.

## R10 – Display-Schnittstelle

- **Decision**: Stecker **IDC 2×5**: 3,3 V, SCK, MOSI, CS, DC, RST, Hintergrundbeleuchtung und
  drei GND zwischen den schnellen Leitungen. 33 Ω Serienwiderstand an SCK und MOSI nahe am
  Controller. SPI2 über IO-MUX, Start mit **26,7 MHz** (80/3 MHz); höher nur, wenn die Anzeige
  am 20-cm-Kabel fehlerfrei bleibt. Hintergrundbeleuchtung: AO3401A an 3,3 V, Gate mit
  100-kΩ-Pull-up (beim Start dunkel), PWM über LEDC.
- **Rationale**: Masseadern und Serienwiderstände dämpfen Reflexionen; reduzierter Takt reicht
  für 240 × 320 bei Teilaktualisierung.

## R11 – Summer und Diagnose-LED

- **Decision**: Passiver Magnetsummer 5 V (≤ 30 mA, Betriebstemperatur ≥ 70 °C prüfen) über
  IRLML6346 an GPIO 35 mit Freilaufdiode, Ton per PWM. Diagnose-LED (0805 oder 3 mm THT, grün)
  mit Vorwiderstand an GPIO 2, aktiv High. Debug-UART: 3-polige Pinleiste (GPIO 43/44, GND),
  optional bestückt.
- **Rationale**: Passiver Summer erlaubt unterschiedliche Töne für Hinweise und Fehler.

## R12 – Revisionserkennung

- **Decision**: Spannungsteiler aus zwei 1-%-Widerständen zwischen 3,3 V und GND an GPIO 1
  (ADC1). REV A = 10 kΩ / 10 kΩ → 1,65 V. Die Firmware liest beim Start mehrfach, mittelt und
  ordnet nach Fenstern zu (REV A: 1,45–1,85 V; außerhalb → unbekannt). Compile-Zeit-Board und
  erkannte Revision müssen übereinstimmen, sonst E204 und sicherer Zustand.
- **Rationale**: Ein ADC-Pin codiert viele künftige Revisionen; Fenster weit genug für
  ADC-Ungenauigkeit des ESP32-S3.
- **Alternatives considered**: Strapping über mehrere Pins (mehr GPIOs); nur Compile-Zeit
  (erkennt falsch geflashte Firmware nicht).

## R13 – Layout: Lagen, Zonen und Platzierung

- **Decision**:
  - **2 Lagen**, FR-4, 1,6 mm, 35 µm Kupfer; Unterseite durchgehende GND-Fläche im
    Kleinspannungsbereich; alle SMD-Teile oben (FR-033).
  - Zielgröße **≤ 100 × 100 mm** (günstigste Fertigungsstufe), Grenze 150 × 150 mm.
  - Zonen von einer Kante zur anderen: **Netz** (Klemme, Sicherung, Varistor, IRM-10-5) →
    Schlitz → **Versorgung** (LDO, Hotspot mit TC6501) → **Logik** (ESP32-S3 an der Kante mit
    U.FL, Flipflop, Watchdog) → **Messung** (MAX31856 + Buchsen an der Kante zur Heizkammer).
  - Alle Steckverbinder am Rand, gruppiert: Netz und Lastteil an einer Kante, Display,
    Bedienteil, USB, Fühler an den übrigen; Beschriftung mit Funktion und Pinbelegung.
  - **Montagelöcher M3 ohne Kupfer**, Abstandshalter aus Kunststoff oder Metall mit isolierender
    Scheibe: Kleinspannungs-GND darf nicht über das Montageblech an PE (FR-029).
  - **Testpunkte** (Lötaugen bzw. Testösen) für 5 V, 3,3 V, GND, Q, Löschleitung, ARM, HEAT_TOP,
    HEAT_BOT, SSR-Versorgung, WDI; keine im Netzbereich (FR-034).
  - Silkscreen: „REV A", Projektname, Warnhinweis „230 V" im Netzbereich.
- **Rationale**: 2 Lagen reichen bei dieser Signaldichte und SPI-Takten; die Reihenfolge der
  Zonen trennt Netz und Messung maximal und hält die Wärmequellen von den Buchsen fern.

## R14 – Versorgungspfade 5 V und 3,3 V

- **Decision**: IRM-10-5 und USB-VBUS speisen die 5-V-Schiene je über eine Schottky-Diode
  **SS34** (Dioden-ODER). 3,3 V aus **AP7361C-33** (SOT-223, Kupferfläche ≥ 4 cm²),
  Ein- und Ausgang 10 µF Keramik. Die 5-V-Schiene liegt damit bei ca. 4,5 V; das reicht für LDO
  (Dropout ≤ 0,4 V), Relais (Anzug ≤ ca. 3,75 V, prüfen) und SSR-Eingänge (≥ 3 V).
- **Rationale**: Ohne Diode im Netzteilzweig würde USB rückwärts in den Ausgang des IRM-10-5
  speisen; mit beiden Dioden ist die Rückspeisung in beide Richtungen ausgeschlossen (FR-003).
  Ergänzt die Beschreibung in `docs/hardware-umbau.md` (dort nur Diode im USB-Zweig).

## R15 – Pinbelegung REV A

- **Decision**: Belegung wie in `docs/hardware-umbau.md` („Belegung REV A (Entwurf)"), ergänzt um
  **CLR_SENSE an GPIO 36** (R6). Vollständige Tabelle in
  [contracts/connectors.md](contracts/connectors.md#pinbelegung-esp32-s3-rev-a). Prüfung der
  Reset-/Boot-Zustände und Glitches gegen ESP32-S3 Datasheet v2.2 ist dokumentiert; GPIO 36 hat
  keinen Glitch und keinen Pull beim Reset.

## R16 – Firmware-Struktur für REV A

- **Decision**:
  - Neue Umgebung `esp32s3_reva` (`-DOVEN_BOARD_REV_A`); `esp32_revdev` bleibt baubar.
  - `temperature`: neues Interface `IThermocoupleReader` (liefert `TemperatureReading`);
    `OvenTemperatureMonitor` hängt davon ab. Implementierungen: `Max31855Reader`
    (Rohwort + bestehender Decoder) und `Max31856Reader` (Register-Bus + neuer `Max31856Decoder`).
    Zwei Monitore (oben, unten).
  - Neues Modul `heater` mit `IHeaterOutputs`, `IEnableLatch`, `IStopButton`,
    `IExternalWatchdog` und `SafetyHeartbeat`.
  - Neues Modul `board` mit `IRevisionStrap`, `RevisionDetector`, `PcbTemperatureMonitor`.
  - Start: erste Aktion DISARM; Heizausgänge aus; Revision prüfen; dann wie Feature 001.
  - Die Inbetriebnahme-Firmware setzt die Freigabe **nie** im Normalbetrieb, nur in HIL-Tests.
  - Version 0.2.0.
- **Rationale**: Feature 001 läuft unverändert weiter (FR-035); neue Hardware nur über
  Interfaces (Prinzip I/III/VI).

## R17 – Temperaturklassen der Bauteile (FR-030)

| Bauteil | Zulässige Temperatur | Bewertung |
|---|---|---|
| IRM-10-5 | 70 °C (bzw. 85 °C neuere Revision) Umgebung | Auslegungsgrenze |
| ESP32-S3-WROOM-1U-N8 | 85 °C | ✅ |
| MAX31856, TPS3828, SN74LVC74A, AP7361C, USBLC6-2SC6 | ≥ 85 °C | ✅ |
| TC6501 | 125 °C | ✅ |
| MOSFETs, Dioden | ≥ 150 °C Sperrschicht | ✅ |
| Elkos | 105-°C-Typen wählen | Auswahlregel |
| Steckverbinder, Klemmen, USB-C | ≥ 85 °C (prüfen) | Auswahlregel |
| Summer | ≥ 70 °C (prüfen) | Auswahlregel |

## Stückliste (Vorauswahl, Hauptplatine)

| Funktion | Bauteil | Gehäuse | Quelle |
|---|---|---|---|
| Netzteil | Mean Well IRM-10-5 | Modul THT | Reichelt |
| Sicherung | T 500 mA | TR5 / 5×20 | offen |
| Varistor | S07K275 | THT | offen |
| LDO 3,3 V | AP7361C-33E | SOT-223 | offen |
| Dioden-ODER | SS34 (2×) | SMA | Magazin |
| Controller | ESP32-S3-WROOM-1U-N8 | Modul | Benutzer bestellt |
| USB-C | 16-polige Buchse mit THT-Laschen | – | offen |
| ESD | USBLC6-2SC6 (3×) | SOT-23-6 | Magazin |
| Thermoelement | MAX31856 (2×) | TSSOP-14 | offen |
| Fühlerbuchse | Mini-TC Typ K, Print (2×) | THT | offen |
| Flipflop | SN74LVC74A | SOIC-14 | offen |
| Watchdog | TPS3828-33 | SOT-23-5 | offen |
| Thermostat | TC6501P065 | SOT-23-5 | offen |
| MOSFET N | IRLML6346 (4×: 2 × SSR, Relais, Summer) | SOT-23 | Magazin |
| MOSFET N | BSS138 | SOT-23 | Magazin |
| MOSFET P | AO3401A (2×: SSR-Versorgung, Beleuchtung) | SOT-23 | Magazin |
| Freilauf | MBR0530 (2×) | SOD-123 | Magazin |
| Summer | passiv, 5 V | THT | offen |
| LED | grün | 0805 / 3 mm | offen |
| Stecker | Klemmen 5,08 mm (2-pol.), 3,81 mm (6-pol.), IDC 2×5, IDC 2×4, Pinleiste 3-pol. | THT | offen |
| Taster | BOOT, RESET | THT 6 × 6 mm | offen |
| Passive | R, C 0603/0805 | SMD | Magazin (Ausnahmen: 1 MΩ, 4,7 nF SMD) |
