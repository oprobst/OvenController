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

- **Änderung (2026-10-25, nach T020)**: Der aus SKiDL erzeugte KiCad-Schaltplan war elektrisch
  korrekt, aber kaum lesbar. Für die Sicherheitsprüfung (T024) braucht es einen lesbaren
  Schaltplan. Deshalb ist ab jetzt der **KiCad-Schaltplan die Quelle**: einmal aus SKiDL als
  Vorlage erzeugt (Blätter Netzteil, MCU, USB, Thermoelemente, Sicherheit, Last, Bedienung), in
  KiCad aufgeräumt und gepflegt. Die geprüfte SKiDL-Netzliste ist als `controller.ref.net`
  eingefroren; `hardware/tools/check_schematic.py` sichert die Übereinstimmung ab. Beim Erzeugen
  hat dieses Skript zwei Generatorfehler gefunden (kurzgeschlossene Netze durch überlappende
  Symbole, Zeichensatz der MPN-Felder); verwendet wird nur eine Vorlage ohne Verbindungsfehler.

## R2 – Netzeingang, Netzteil und Isolation

- **Decision**:
  - Mean Well **IRM-10-5** (5 V / 2 A, Printmodul).
  - Netzeingang: Schraubklemme 2-polig (ab 2026-10-30 fest statt steckbar), Raster 5,08 mm → Feinsicherung **T 500 mA**
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
  Geprüft in T009: Das aktuelle Datenblatt (IRM-10-SPEC, 2025-08-08) nennt −30…+85 °C; die
  Auslegung auf 70 °C bleibt als Reserve. Maße 45,7 × 25,4 × 21,5 mm.
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
  - Umsetzung (T013): BIAS an T−, 10 nF differenziell zwischen T+ und T−.
  - **Geprüft am Originaldatenblatt (Maxim MAX31856, „Typical Application Circuit", 2026-10-26):**
    BIAS an T− bestätigt. Der Eingangsfilter ist dort **0,1 µF differenziell** zwischen T+ und T−
    plus **je 0,01 µF von T+ und T− nach GND** (Gleichtaktfilter). **Umgesetzt 2026-10-26:** C32/C23
    auf 100 nF, C21, C25, C31, C33 (je 10 nF, T+ bzw. T− nach GND) ergänzt, Referenznetzliste im selben
    Commit neu exportiert. AVDD und DVDD je 0,1 µF bestätigt. SPI: beide Taktpolaritäten, max.
    5 MHz. Betrieb −55…+125 °C.
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
    **Stopp-Öffner** und **4,7 kΩ** (ca. 0,7 mA; der TPS3828 garantiert VOL ≤ 0,45 V nur bis
    1,2 mA, daher nicht 1 kΩ). Pegel bei geschlossenem Öffner: 3,3 V × 100/104,7 ≈ 3,15 V.
    Open-Drain-Quellen direkt auf der Leitung: TPS3828 /RESET und TC6501. Die FAULT-Ausgänge
    beider MAX31856 gehen je über eine **Schottky-Diode MBR0530** (Kathode am FAULT-Pin) auf
    die Leitung. Laut Originaldatenblatt (geprüft 2026-10-26) ist FAULT **Push-Pull** (aktiver
    High-Pegel, kein Open-Drain erwähnt, in der Anwendungsschaltung ohne Pull-up); ohne Diode
    würde FAULT die Löschleitung im Normalzustand gegen die anderen Quellen treiben. Mit der Diode
    funktioniert es (Low-Pegel auf der Leitung ca.
    0,3 V < 0,8 V Eingangsschwelle des 74LVC74). 1 nF gegen GND gegen Störspitzen aus dem Kabel;
    ESD-Schutz am Steckverbinder (R9).
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
- **Datenblatt (TI SLVS165O, Rev. März 2025), geprüft in T006**:
  - Watchdog-Zeit TPS3823/4/8: **0,9 / 1,6 / 2,5 s** (min/typ/max). → SC-002 wird auf
    **≤ 2,6 s** angepasst. Herzschlag alle 250 ms liegt weit unter dem Minimum von 0,9 s.
  - Reset-Verzögerung 120 / 200 / 300 ms.
  - **Offener WDI schaltet den Watchdog ab** („If left floating, the device generates pulses
    internally"). WDI zieht bis zu 190 µA. → **Pull-down 2,2 kΩ an WDI ist Pflicht**
    (0,42 V < 0,3 × VDD). Damit läuft der Watchdog auch, wenn der ESP32 im Reset ist oder
    bootet; die Freigabe bleibt dann gelöscht, wie gewollt.
  - /MR: interner Pull-up 90 kΩ, Mindestpulsbreite 1 µs → DISARM-Puls 100 µs.
  - Schwelle -33: 2,83 / 2,93 / 3,00 V; Ausgang Open-Drain, VOL ≤ 0,45 V bei 1,2 mA.
  - Temperaturbereich −40…+85 °C.
- **Alternative geprüft**: TPS3820-33 (Watchdog 112–300 ms) hat einen Push-Pull-Ausgang und
  würde einen Herzschlag < 100 ms verlangen; verworfen.
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
  - Relaisspule: IRLML6346 vom Latch-Ausgang Q, Freilaufdiode MBR0530.
- **Relais (geprüft in T007, Finder Datenblatt Serie 40, VIII-2026)**:
  - **40.61.9.005.0000** (Standardspule 0,65 W): 38 Ω, 130 mA, Anzug ab **3,65 V**, max. 7,5 V.
    Bei ca. 4,5 V Schiene bleibt Reserve auch für die Erwärmung der Spule.
  - Nicht die empfindliche Spule 40.61.7.005: für 40.61 Anzug erst ab 0,8 × UN = 4,0 V.
  - Kontakt 16 A / 250 V, AC1 4000 VA, Umgebung −40…+85 °C.
  - Fassung **95.05**: 10 A / 250 V, −40…+70 °C (über 10 A Kontakte parallel). Bei 5,2 A
    reicht ein Kontakt. Freilaufmodul 99.02.3.000.00 nicht nötig (Diode auf der Platine).
- **SSR**: **Crydom D2425** (Nullspannungsschalter, 25 A, 24–280 V AC), Steuereingang
  3–32 V DC, max. 12 mA (Distributorangaben). Bei ca. 4,4 V an `+5V_SSR` ausreichend. Nicht
  Carlo Gavazzi RM1A23D25: Steuereingang erst ab 4,25 V, zu knapp.
  - Lastteil-Anschluss 6-polig, Schraubklemme Raster 3,81 mm (ab 2026-10-30 fest statt steckbar) (Belegung:
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
  - Hauptplatine: **JST XH 1×5** (J4) für den Encoder (A, GND, B, Taster, GND) und **JST XH
    1×4** (J5) für die Stopp-Taste (Öffner Hin- und Rückleitung, Schließer, GND), stehend
    (B5B-/B4B-XH-A). Entscheidung 2026-10-29 (vorher IDC 2×4 für beides): eine Buchse je
    Bedienelement, verschiedene Polzahlen gegen Verwechseln; ein abgezogener Stopp-Stecker öffnet
    den Öffner-Kreis und wirkt wie eine gedrückte Stopp-Taste.
  - Eingänge mit 10 kΩ Pull-up, 10 kΩ Serie, 10 nF nach GND (Datenblatt Alps EC11E); ESD-Schutz
    mit **USBLC6-2SC6** (VBUS-Pin an 3,3 V). Ein Baustein schützt **zwei** Leitungen (Pins 1/6
    und 3/4 sind jeweils dieselbe Leitung): zwei Bausteine für ENC_A/ENC_B und ENC_SW/STOP_NO,
    ein dritter für die beiden Stopp-Öffner-Leitungen (Korrektur in T016).
  - `STOP_NC_OUT` über 220 Ω an 3,3 V, damit ein Kurzschluss im Kabel den LDO nicht belastet
    (Pegel an CLR bei geschlossenem Öffner weiterhin ca. 3,1 V).
  - **Encoder-Träger** (eigene kleine Platine, ca. 25 × 30 mm): Alps EC11E15244G1, JST XH 1×5
    zur Hauptplatine. Befestigung nur über die Mutter des Encoders.
  - Schritte pro Rastung: 2 (15 Impulse / 30 Rastungen) in `board_rev_a.h`.
- **Rationale**: ESD-Schutz mit vorhandenem Baustein; Entprellung in Hardware entlastet die
  Interrupts; der Träger hält den Encoder mechanisch sauber in der Front.

## R10 – Display-Schnittstelle

- **Decision**: Stecker **JST XH 1×10** (stehend, B10B-XH-A; bis 2026-10-29 IDC 2×5, gleiche
  Pin-Reihenfolge): 3,3 V, SCK, MOSI, CS, DC, RST, Hintergrundbeleuchtung und drei GND zwischen
  den schnellen Leitungen; im Kabel die Adern in Pin-Reihenfolge eng führen. 33 Ω Serienwiderstand an SCK und MOSI nahe am
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
| Lüfter Noctua NF-A4x10 5V PWM | 70 °C (Datenblatt) | ✅ Grenze, sitzt im Frischluftstrom |
| Lüfterstecker Molex 47053-1000 | 0…+75 °C (Avnet), −40…+105 °C (Arrow); Molex-Zeichnung bei der Bestellung prüfen | ✅ beide ≥ 70 °C |
| Netzabdeckung PC | Glasübergang ca. 110–145 °C | ✅ |

## R18 – Platinenlüfter (FR-036 bis FR-038)

Quelle: Noctua PWM specifications white paper (gilt für alle 4-Pin-Lüfter mit 5 V, 12 V und
24 V von Noctua) und Produktdaten NF-A4x10 5V PWM.

- **Decision**:
  - **Lüfter**: Noctua NF-A4x10 5V PWM in der Gehäusewand, bläst Außenluft über die Platine.
    5 V, typisch 0,3 W / 0,06 A, maximal 0,35 W / 0,07 A (Noctua-Datenblatt), 0–5000 U/min,
    ca. 1050 U/min bei 20 % PWM, Betrieb −10…+70 °C. Geprüft in T072 (2026-10-26).
  - **Stecker J9**: 4-poliger Lüfter-Wannenstecker Molex 47053-1000 (Raster 2,54 mm, mit
    Verpolschutz-Steg), Footprint `Connector:FanPinHeader_1x04_P2.54mm_Vertical`. Belegung nach
    Intel-4-Draht-Standard: 1 GND, 2 +5V, 3 Drehzahl, 4 PWM. Der Lüfter steckt direkt oder mit
    Verlängerung NA-SEC1 (30 cm).
  - **Versorgung**: Lüfter dauerhaft an +5V (Dioden-ODER-Schiene, ca. 4,5 V); 10 µF und 100 nF
    am Stecker. Der Lüfter läuft auch bei reiner USB-Versorgung.
  - **PWM**: GPIO 38 → N-MOSFET BSS138 (Gate 100 kΩ nach GND) → PWM-Pin; **1 kΩ Pull-up nach
    +5V** am Drain. 25 kHz über LEDC mit invertiertem Ausgang (GPIO high = PWM low). Das
    Whitepaper verlangt VIL ≤ 0,8 V, max. 5,25 V und rät von Open-Collector-Stufen ohne
    externen Pull-up ab, weil der interne Pull-up des Lüfters schwach ist; mit 1 kΩ steigt die
    Flanke in ca. 1 µs (Periode 40 µs). Kein direkter Anschluss an den GPIO: der interne Pull-up
    des Lüfters zieht bis 5 V, der ESP32-S3 verträgt nur 3,6 V.
  - **Fail-safe**: Im Reset, beim Booten und bei hochohmigem GPIO sperrt der MOSFET; der PWM-Pin
    liegt dann auf 5 V und der Lüfter läuft mit voller Drehzahl (Whitepaper: ohne PWM-Signal
    volle Drehzahl).
  - **Drehzahl**: Open-Collector-Ausgang, 2 Pulse je Umdrehung, max. 5 mA. 10 kΩ Pull-up nach
    3,3 V, 1 kΩ Serie und 1 nF nach GND vor GPIO 37; Zählung mit dem Pulszähler (PCNT).
    5000 U/min = 167 Hz. Blockierter Rotor und abgezogener Stecker liefern einen festen Pegel,
    also 0 Hz.
  - **Testpunkt** TP11 FAN_TACH (Kleinspannung) für die Inbetriebnahme.
  - **Firmware**: neues Modul `cooling` (siehe [contracts/interfaces.md](contracts/interfaces.md#cooling-neu)).
    Kennlinie nach FR-038: 20 % bis 35 °C, linear bis 100 % bei 50 °C; Platinentemperatur
    ungültig (E202) → 100 %. Überwachung nach FR-037: Ausfall, wenn die Drehzahl 3 s lang
    unter 500 U/min liegt (Messfenster 1 s, Erkennung ≤ 4 s < SC-009), neuer Fehlercode
    **E206 `FanFailure`**: DISARM, verriegelnder Fehler, Anzeige auch ohne Heizvorgang.
- **Rationale**: Ein MOSFET mit externem Pull-up ist die einfachste Stufe, die den Pegel des
  Lüfters sauber auf 5 V hebt, den GPIO schützt und bei stehender Firmware volle Kühlung
  liefert. BSS138 und die passiven Bauteile sind im Magazin. Die Mindestdrehzahl von 20 % liegt
  an der unteren Grenze des Intel-Standards; darunter bleibt der Noctua stehen, die Kennlinie
  geht deshalb nie unter 20 %. Die Ausfallschwelle von 500 U/min liegt mit Abstand unter
  1050 U/min bei 20 %.
- **Alternatives considered**: GPIO direkt an PWM (Überspannung am GPIO durch den Pull-up des
  Lüfters); Pegelwandler 74AHCT1G125 an 5 V (sauberer Gegentakt, aber zusätzliches Bauteil und
  ohne Pull-up kein definierter Zustand im Reset); Lüfterversorgung über Low-Side-MOSFET
  (3-Draht-Steuerung, verfälscht das Drehzahlsignal); eigene Platinentemperatur-Messung für den
  Lüfter (die CJ-Werte der MAX31856 reichen, R5).
- **Freie Pins danach**: 3, 45, 46 (Strapping, nicht beschalten) und 48.

## R19 – Netzabdeckung (FR-039, SC-010)

Ausgangslage (Vorplatzierung T020): Der Netzbereich bildet ein L am linken und unteren Rand –
Spalte mit J1, F1 und RV1 (x ≈ 0–17 mm, y ≈ 9–64 mm) und das IRM-10-5 unten links
(x ≈ 3–49 mm, y ≈ 65–91 mm, Höhe 21,5 mm, Frässchlitz bei x = 25–27 mm unter dem Modul). Die
nächsten Kleinspannungsbauteile beginnen bei x ≈ 23 mm.

- **Decision**:
  - **Umfang**: Die Abdeckung umschließt das ganze L einschließlich des **gesamten IRM-10-5**
    (eine Wand quer über das Modul ist nicht dicht herstellbar; das gekapselte Modul ist auf
    der Sekundärseite ohnehin berührungssicher).
  - **Aufbau**: Ober- und Unterschale aus PC, Wandstärke 2 mm, 100 % Füllung in Wänden und
    Deckel. An den Platinenkanten (links, unten) greifen die Schalen **um die Kante herum** und
    überlappen sich (Stufenfuge), sodass auch die Stirnseite der Platine abgedeckt ist; die
    Schalen ragen dort ca. 3 mm über den Platinenrand. Innen, zur Kleinspannungsseite, steht die
    Wand auf der Platine, und zwar im kupferfreien Isolationsband zwischen Netz- und
    Kleinspannungskupfer (6 mm laut R2): Wand 2 mm + je 0,5 mm Toleranz.
  - **Spalte**: Konstruktiv ≤ 0,5 mm (Auflage der Wand auf der Lötstopplackseite, Stufenfuge an
    der Kante, Kabeldurchführung); damit bleibt Reserve zur Grenze von 1 mm und der Prüfdraht
    nach SC-010 kommt nicht hindurch. Keine Lüftungsschlitze in der ersten Version (siehe
    Wärme).
  - **Innenhöhe** oben 25 mm (IRM-10-5 21,5 mm, J1 mit Stecker und Kabelbogen prüfen),
    Gesamthöhe ca. 27 mm über der Platine (≤ 40 mm laut FR-028). Unten 4 mm Innenhöhe für
    Lötstellen und Drahtenden (THT-Beinchen auf ≤ 2 mm kürzen), ca. 6 mm unter der Platine.
  - **Befestigung**: 4 Bohrungen **H5–H8** (Ø 3,2 mm, ohne Kupfer) mit durchgehenden
    Polyamid-Schrauben M3 und Muttern; Abstand zum Netzkupfer ≥ 6 mm (eigene DRC-Regel),
    Abstand zwischen zwei Schrauben entlang einer Wand ≤ 50 mm, damit sich keine Kante um mehr
    als 0,5 mm abheben kann. Die Bohrungen liegen auf der Kleinspannungsseite direkt neben der
    Innenwand oder in den Schalenecken außerhalb des Kupfers.
  - **Montagelöcher H1/H3** liegen im Abdeckbereich: Beide Schalen bekommen dort geschlossene
    Hülsen, die dicht auf der Platine aufsitzen; die Befestigungsschraube der Platine läuft
    durch die Hülse, ein Prüfdraht in der Hülse erreicht nur das kupferfreie Bohrloch (Regel
    „mains_to_mounting_holes“, ≥ 3 mm). Alternativ verlegt der Benutzer H1/H3 beim Abstimmen
    mit dem Gehäuse aus dem Abdeckbereich; die Hülsen entfallen dann.
  - **Netzkabel**: Mantelleitung H05VV-F 2 × 0,75 mm² (Außendurchmesser am Muster messen,
    typisch ca. 6 mm). Durchführung in der linken Wand als Rundloch aus zwei Halbschalen
    (Durchmesser = gemessener Kabeldurchmesser + 0,3 mm), Mantel endet mindestens 5 mm innerhalb
    der Abdeckung. Zugentlastung: Kabelbinder-Steg an der Unterschale außen. Die Klemme J1
    liegt ganz innen; Adern bei offener Abdeckung anklemmen.
  - **Werkstoff**: PC-Filament (Glasübergang ca. 110–145 °C, weit über 70 °C), bevorzugt eine
    flammgeschützte Sorte (UL 94 V-0), sonst Standard-PC; das Metallgehäuse bleibt die
    Brandschutzhülle.
  - **Wärme**: IRM-10-5 bei ca. 0,5 A Last (R2 plus Lüfter) mit ca. 0,8 W Verlust. Erste
    Version geschlossen; Abnahme misst die Gehäusetemperatur des Moduls unter der Abdeckung
    bei Dauerlast (60 min). Liegt die Erwärmung gegenüber der Luft außerhalb der Abdeckung über
    10 K, bekommt der Deckel Lüftungslamellen als Labyrinth (überlappend, kein gerader Weg zu
    Netzspannung führenden Teilen).
  - **Abbildung im Layout**: Board-only-Footprint **COV1** (`oven:Netzabdeckung_REVA`, nicht in
    der Stückliste) mit Umriss der Schalen auf F.Fab/B.Fab, Sperrflächen (Rule Areas: keine
    Footprints) entlang der Innenwand auf beiden Seiten, den NPTH-Bohrungen H5–H8 als Pads und
    dem 3D-Modell der Schalen. DRC-Regel `cover_holes_to_mains` (`physical_hole_clearance`, weil `hole_clearance` NPTH-Bohrungen nicht prüft): Bohrungen von COV1 ≥ 6 mm
    zu Netzklasse `Mains`. Bauteilhöhen unter dem Deckel ≤ 25 mm.
  - **Modell**: Fusion 360 (Prinzip X), Quelle und Exporte unter `hardware/controller/cover/`
    (`netzabdeckung.f3d`, `netzabdeckung_oben.step/.stl`, `netzabdeckung_unten.step/.stl`,
    `README.md` mit Druckeinstellungen). Konstruiert gegen den STEP-Export der Platine; der
    Umriss in COV1 und das Fusion-Modell werden gegeneinander geprüft.
- **Rationale**: Ein geschlossenes Gehäuse aus zwei Schalen ist mit FDM sicher dicht
  herstellbar, Stufenfugen und kleine Spalte halten den Prüfdraht zuverlässiger ab als
  Lüftungsschlitze. Die Innenwand im ohnehin kupferfreien Isolationsband kostet keine
  Platinenfläche. Kunststoffschrauben bringen kein leitfähiges Teil an die Oberfläche. Ein
  Footprint macht die Abdeckung für DRC, 3D-Ansicht und STEP-Export sichtbar, sodass Bauteile
  unter der Wand und zu hohe Teile auffallen.
- **Alternatives considered**: Nur Oberschale (Lötstellen unten offen, verworfen in der Spec);
  Rastnasen (ohne Werkzeug lösbar, bruchgefährdet); Metallschrauben (leitfähig an der
  Oberfläche); Wand quer über das IRM-10-5 (nicht dicht); Lüftungsschlitze von Anfang an
  (Schlitze < 1 mm im FDM-Druck unzuverlässig, erst nach Messung).
- **Maße (T082, 2026-10-26)**:
  - IRM-10-5: 45,7 × 25,4 × 21,5 mm (IRM-10-SPEC 2025-08-08). Netzseitige Pins 1 (AC/N) und
    2 (AC/L) am linken Modulende (x = 6,75 mm), Ausgang 3/4 am rechten (x = 45,25 mm); das
    Modul ist damit das höchste Teil unter dem Deckel.
  - J1 MSTBA 2,5/2-G-5,08 (horizontal): 8,6 mm hoch (KiCad-3D-Modell); Stecker
    MSTB 2,5/2-ST-5,08 laut Datenblatt 15 mm in der größten Abmessung, steckt waagerecht.
    Mit Kabelbogen ≤ 15 mm; am Muster bestätigen. Ab 2026-10-30 ersetzt durch die feste Klemme
    MKDS 1,5/2-5,08 (kein Stecker mehr, Drahteinführung zur linken Kante); Höhe am Muster messen.
  - F1 Schurter OGN 0031.8201 (offener Halter, ab 2026-10-29): max. 11,5 mm hoch (Distributor-Angabe).
  - RV1 S07K275: Scheibe stehend, ca. 10–13 mm (am Bauteil messen).
  - Alle Teile unter dem Deckel ≤ 21,5 mm: Innenhöhe 25 mm passt.
  - Netzkabel H05VV-F 2 × 0,75 mm²: Außendurchmesser 5,7–7,2 mm je Hersteller (Helukabel,
    Nexans). Die Durchführung wird deshalb erst nach Messung am gekauften Kabel festgelegt
    (Durchmesser + 0,3 mm).
  - Filament: PC-FR mit V-0, z. B. Elegoo PC-FR oder Bambu Lab PC FR (UL 94-2023 V-0 laut
    Händler), Polymaker PolyMax PC-FR (V-0 nach IEC 60695-11-10, Makrolon-Basis). Alle
    brauchen einen geschlossenen Drucker (Düse 250–280 °C, Bett 90–110 °C); Prüfbericht beim
    Kauf anfordern. Ohne V-0 ist Standard-PC zulässig (R19, Werkstoff).
- **Folgen für das Layout (vor T021)**: J10/U12 weiter vom IRM-10-5 abrücken (ohnehin offen in
  T020, Wärme), damit Wand und Überstand an der rechten Modulkante Platz haben; die
  Kleinspannungsbauteile an der Innenwand (J2, Q1–Q3, U3, C2, C1) so weit nach rechts, dass
  Sperrfläche und H5–H8 passen.
- **Ergebnis T084 (2026-10-27)**: COV1 liegt im Ursprung der Platine; die Wandlage aus T083
  passt unverändert. **H8 liegt innen** bei (46,5 / 96) unter dem IRM-10-5, als Dom an der
  Innenseite der rechten Wand: Außen hätte der Flansch die Thermoelement-Buchsen blockiert, weil
  J10 und J8 nebeneinander die ganze Unterkante zwischen Wand und H4 brauchen. Abstand zum
  Netzkupfer > 30 mm. H5–H7 bleiben außen. Verschoben: J2 (+2 mm), U1, U4 (TC6501) und J10/J8
  (+1,8 / +0,6 mm); J10 liegt 4 mm neben dem IRM-10-5, dazwischen die Wand. Q1–Q3, U3, C2 und C1
  mussten nicht verschoben werden. DRC mit Schaltplanabgleich: keine Verletzung durch COV1; die
  Gegenprobe (J10 in die Wand geschoben) meldet `items_not_allowed`.

## Stückliste (Auswahl T008, 2026-10-25)

Hersteller-Teilenummern sind gewählt. „prüfen" heißt: Teilenummer, Footprint und Preis beim
Anlegen des Symbols (T009) bzw. bei der Bestellung (T025) bestätigen. Preise sind Richtwerte
pro Stück inkl. MwSt.

### Hauptplatine

| Funktion | Hersteller / Teilenummer | Footprint (KiCad 10) | Quelle | Preis |
|---|---|---|---|---|
| Netzteil | Mean Well IRM-10-5 | eigener (hardware/lib) | Reichelt IRM-10-5 | 8,10 € |
| Sicherung | Feinsicherung 5 × 20 mm T 500 mA + offener Leiterplatten-Sicherungshalter Schurter OGN 0031.8201 (Raster 22,5 mm, Sicherung wird von oben eingedrückt; ersetzt 2026-10-29 den geschlossenen FAB, dessen Einsatz axial gezogen wird und dafür ca. 25 mm Platz vor der Kappe braucht, den RV1 und das IRM-10-5 versperren). Berührschutz über die Netzabdeckung; eine Abdeckkappe (Schurter 0853.0551, ca. 15 mm breit) ist optional und müsste gegen die Wand der Abdeckung geprüft werden | `Fuse:Fuseholder_Cylinder-5x20mm_Schurter_0031_8201_Horizontal_Open` | Distributor (prüfen) | prüfen |
| Varistor | TDK/EPCOS S07K275 (B72207S2271K101) | `Varistor:` Scheibe 7 mm | Reichelt | ca. 0,30 € |
| LDO 3,3 V | Diodes AP7361C-33E-13 | `Package_TO_SOT_SMD:SOT-223-3_TabPin2` (Pinbelegung prüfen) | Mouser | ca. 0,50 € |
| Dioden-ODER | SS34 (2×) | `Diode_SMD:D_SMA` | Magazin | – |
| Controller | Espressif ESP32-S3-WROOM-1U-N8 | `RF_Module:ESP32-S3-WROOM-1U` | Benutzer bestellt | ca. 4 € |
| USB-C | GCT USB4085-GF-A (Durchsteckmontage, USB 2.0) | `Connector_USB:USB_C_Receptacle_GCT_USB4085` | Mouser | ca. 1 € (prüfen) |
| ESD | ST USBLC6-2SC6 (4×: USB, 3 × Bedienteil) | `Package_TO_SOT_SMD:SOT-23-6` | Magazin (Bestand prüfen) | – |
| Thermoelement | Analog Devices MAX31856MUD+ (2×) | `Package_SO:TSSOP-14_4.4x5mm_P0.65mm` | Mouser 700-MAX31856MUD+ | 12,26 € (2026-10-26) |
| Fühlerbuchse | RS PRO 381-7564 (Print, Typ K, IEC grün, Kontakte NiCr/Ni, bis 220 °C), ersetzt Omega PCC-SMP-K (46 € je 5er-Pack) am 2026-10-27 | eigener (hardware/lib) | RS 381-7564 | 5,27 € netto (2026-10-27) |
| Flipflop | TI SN74LVC74AD | `Package_SO:SOIC-14_3.9x8.7mm_P1.27mm` | Mouser | ca. 0,50 € |
| Watchdog | TI TPS3828-33DBVR | `Package_TO_SOT_SMD:SOT-23-5` | Mouser | ca. 1 € |
| Thermostat | Microchip TC6501P065VCTTR | `Package_TO_SOT_SMD:SOT-23-5` | DigiKey 412996 | ca. 0,75 € |
| MOSFET N | IRLML6346 (4×) | `Package_TO_SOT_SMD:SOT-23` | Magazin | – |
| MOSFET N | BSS138 | `Package_TO_SOT_SMD:SOT-23` | Magazin | – |
| MOSFET P | AO3401A (2×) | `Package_TO_SOT_SMD:SOT-23` | Magazin | – |
| Schottky klein | MBR0530 (4×: Relais, Summer, FAULT oben/unten) | `Diode_SMD:D_SOD-123` | Magazin (Bestand prüfen) | – |
| Summer | passiver Magnet-Summer 5 V, ≤ 30 mA, Betriebstemperatur ≥ 70 °C | `Buzzer_Beeper:` (prüfen) | offen | ca. 1 € |
| Lüfterstecker J9 | Molex 47053-1000 (4-polig, 2,54 mm, mit Steg) | `Connector:FanPinHeader_1x04_P2.54mm_Vertical` | Mouser/Reichelt | ca. 0,50 € (prüfen) |
| Lüfter-Ansteuerung | BSS138 (2. Stück), 1 kΩ Pull-up an +5V, 100 kΩ Gate; Drehzahl 10 kΩ / 1 kΩ / 1 nF; 10 µF + 100 nF am Stecker | `Package_TO_SOT_SMD:SOT-23`, 0603/0805 | Magazin | – |
| LED | grün, 0805 | `LED_SMD:LED_0805_2012Metric` | Mouser/Reichelt | ca. 0,10 € |
| Netzklemme | Phoenix MKDS 1,5/2-5,08 (feste Schraubklemme, ersetzt MSTBA/MSTB-Stecker am 2026-10-30) | `oven:Phoenix_MKDS-1,5-2-5.08_Horizontal_OvalPads` | Reichelt/Mouser | prüfen |
| Lastteil-Klemme | Phoenix MKDS 1/6-3,81 (feste Schraubklemme, ersetzt MC-Stecker am 2026-10-30) | `oven:Phoenix_MKDS-1-6-3.81_Horizontal_OvalPads` | Reichelt/Mouser | prüfen |
| Display-Stecker | JST XH 1×10 stehend B10B-XH-A + Gehäuse XHP-10 + Crimpkontakte SXH-001T-P0.6 | `Connector_JST:JST_XH_B10B-XH-A_1x10_P2.50mm_Vertical` | Reichelt/LCSC | prüfen |
| Encoder-Stecker | JST XH 1×5 stehend B5B-XH-A (Hauptplatine und Encoder-Träger) + XHP-5 + SXH-001T-P0.6 | `Connector_JST:JST_XH_B5B-XH-A_1x05_P2.50mm_Vertical` | Reichelt/LCSC | prüfen |
| Stopp-Stecker | JST XH 1×4 stehend B4B-XH-A + XHP-4 + SXH-001T-P0.6 | `Connector_JST:JST_XH_B4B-XH-A_1x04_P2.50mm_Vertical` | Reichelt/LCSC | prüfen |
| Debug-UART | Stiftleiste 1×3, 2,54 mm (optional) | `Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical` | Magazin/Reichelt | – |
| Taster | Kurzhubtaster 6 × 6 mm THT (2×) | `Button_Switch_THT:SW_PUSH_6mm` | Reichelt | ca. 0,20 € |
| Elkos | 100 µF / 16 V, 105 °C | THT oder SMD (Magazin nur 16 V ohne Temperaturangabe) | Reichelt | ca. 0,20 € |
| Passive | R, C 0603/0805 | `Resistor_SMD`, `Capacitor_SMD` | Magazin (Ausnahmen: 1 MΩ, 4,7 nF SMD; 2,2 kΩ für WDI vorhanden) | – |

### Außerhalb der Platine

| Funktion | Hersteller / Teilenummer | Quelle | Preis |
|---|---|---|---|
| SSR (2×) | Sensata Crydom D2425 | Mouser/Reichelt | ca. 45–60 € (prüfen) |
| Sicherheitsrelais | Finder 40.61.9.005.0000 | Reichelt/Mouser | ca. 6–10 € |
| Relaisfassung | Finder 95.05 (Schraubmontage) | Reichelt/Mouser | ca. 4–8 € |
| Antenne | U.FL → RP-SMA-Pigtail (ca. 15 cm, Durchführung) + 2,4-GHz-Stabantenne RP-SMA | Reichelt/Amazon | ca. 8 € |
| Platinenlüfter | Noctua NF-A4x10 5V PWM (+ ggf. Verlängerung NA-SEC1) | offen | 15,90 € (2026-10-26) |
| Netzabdeckung | Ober- und Unterschale aus PC-Filament (bevorzugt UL 94 V-0), Druck selbst; 4 + 2 Schrauben M3 Polyamid mit Muttern; Netzkabel H05VV-F 2 × 0,75 mm² | Druck / Reichelt | ca. 5 € |
| Encoder | Alps EC11E15244G1 | Conrad 3523314 | 3,49 € |
| Stopp-Taste | rot, Ø 16–19 mm, 1 Öffner + 1 Schließer | offen | – |
| Thermoelemente (2×) | Typ K, Mantel Ø 1–1,5 mm, isolierte Messstelle, Mini-Stecker SMPW-K | offen | – |
