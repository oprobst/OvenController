# Hardware-Umbau des Ofens

## Ofen: Steinborg SB-3005 (gekauft)

13 L, 1200 W, 65–230 °C, Ober-/Unterhitze, Timer bis 60 min, Sichtfenster.

| Maß | Wert (Herstellerangabe MediaMarkt) |
|---|---|
| Innen (B × T × H) | ca. 26,5 × 22,5 × 17,5 cm |
| Außen (B × T × H) | ca. 38 × 31,5 × 22,5 cm |
| Backblech | ca. 26,5 × 20 × 2 cm |
| Grillrost | ca. 26,5 × 20 cm |
| Kabellänge | ca. 90 cm |

- Eine 15 × 15 cm große Platine passt hinein. Bei mittiger Einschubhöhe bleiben etwa 7–8 cm bis zu
  den Heizstäben.
- 1200 W bei 230 V entsprechen ca. 5,2 A. Für Reflow ist die Leistung gut, zum Halten von 110 °C
  (Silicagel) eher zu hoch. Die Firmware muss die Leistung dort begrenzen.

### Beim Öffnen prüfen und hier eintragen

| Punkt | Befund |
|---|---|
| Anzahl und Anordnung der Heizstäbe (oben/unten getrennt?) | 4 Heizstäbe gesamt, zwei oben und zwei unten. Oben und unten ist getrennt schaltbar. |
| Position Thermostat | _offen_ |
| Position Timer, hat er eine Dauerstellung? | _offen_ |
| Position Thermosicherung | _offen_ |
| Freier Platz hinter der Rückwand | _offen_ (kein Lüfter mehr geplant; Platz für Relais und SSRs) |
| Platz für Controller und SSRs auf der kalten Seite | _offen_ |

## Dämmung (wichtigster Umbauschritt)

- Gehäuse abschrauben und den Garraum außen dämmen: **Keramikfasermatte** 6–13 mm (ca. 0,5 m²)
  oder alukaschierte Glasfaser-Hitzeschutzmatte.
- Keramikfaser mit hitzefestem Alu-Klebeband einhüllen. Beim Verarbeiten Maske und Handschuhe
  tragen (Faserstaub).
- Türspalt mit Glasfaser-Dichtschnur abdichten.
- Optional die Innenwände mit reflektierendem Hitzeschutzband bekleben.
- Bereiche für Kabel, Schalter, Thermostat und SSRs aussparen. Elektronik gehört auf die kalte
  Seite.
- **Kein** Baumarkt-Dämmmaterial mit Bindemittel verwenden.
- Vor dem ersten Löten 30–60 min bei Maximum ausheizen (Ausgasen), dabei gut lüften.

## Verkabelung und Leistungsteil

- Hochtemperatur-Litze: Silikon, nahe den Heizstäben Glasseide.
- Keramik-Lüsterklemmen und unisolierte Kabelschuhe im heißen Bereich.
- Kaltgeräte-Einbaubuchse mit Schalter und Sicherung (z. B. 8 A träge).
- Original-Thermostat und Timer werden überbrückt bzw. auf Maximum/Dauer gestellt.
- Dünnes Alublech (ca. 1 mm, ca. 26,5 × 20 cm) als Platinenträger.

## Komponenten-Vorauswahl Controller

Grobe Kosten: ca. 60–120 €. Das ist eine Vorauswahl. Die endgültige Auswahl erfolgt im Feature
„Controller-Platine“ (`/speckit-plan`).

Preise sind Einzelpreise inkl. MwSt. zum angegebenen Datum; „offen" heißt, das Teil ist noch
nicht konkret ausgewählt.

| Funktion | Vorauswahl | Bezugsquelle | Preis (Stand) |
|---|---|---|---|
| Mikrocontroller | ESP32-S3-WROOM-1-N8 (8 MB Flash, **ohne Octal-PSRAM**), natives USB. Prototyp: ESP32-S3-DevKitC-1-N8. Bisheriger Prototyp REV DEV auf ESP32-DevKitC V4 (Feature 001) (siehe „Mikrocontroller und Pinbelegung") | offen | offen |
| Temperaturmessung | 2 Kanäle (oben, unten) mit je MAX31856 (TSSOP-14, 50-Hz-Filter, FAULT-Ausgang). Prototyp REV DEV: MAX31855-Breakout (siehe „Sensorik") | offen | ca. 8–12 € je IC (Richtwert) |
| Thermoelemente | 2 × Typ K, Mantel Ø 1–1,5 mm, **isolierte Messstelle** | offen | offen |
| Thermoelement-Buchse | 2 × Mini-TC-Buchse Typ K, Printversion, je neben ihrem MAX31856 | offen | offen |
| Heizung schalten | 2 × Panel-SSR (Hockey-Puck), nullspannungsschaltend, 25 A, Markenware (z. B. Crydom D2425, Carlo Gavazzi RM1A, Omron G3NA-225B), nicht auf der Platine. Getrennt für Ober- und Unterhitze (siehe „Lastseite") | offen | ca. 25–45 € je Stück (Richtwert) |
| Sicherheitsrelais | Finder 40.61 (16 A, 1 Schließer genutzt, 5-V-DC-Spule) in Fassung 95.05, angeschraubt (keine Hutschiene) | offen | Relais ca. 5–10 €, Fassung ca. 4–8 € (Richtwert) |
| Externer Watchdog | TI TPS3828-33 (SOT-23-5, Open-Drain-Reset, 1,6 s) in der Löschkette des Freigabe-Latch | offen | ca. 1 € (Richtwert) |
| Freigabe-Latch | D-Flipflop mit asynchronem Löschen, z. B. 74LVC1G74 | offen | offen |
| PCB-Temperatur (Firmware) | Vergleichsstellen-Temperatur (CJ) der MAX31856, kein eigener Sensor | – | – |
| PCB-Übertemperatur (Hardware) | Microchip TC6501 (SOT-23-5, feste Schwelle ab Werk, Open-Drain aktiv low), Schwelle ca. 75 °C, Hysterese 10 °C | offen | ca. 1 € (Richtwert) |
| Lüfter | entfällt (Entscheidung 2026-10-24) | – | – |
| Netzteil | Mean Well IRM-10-5 (230 V → 5 V / 2 A, 10 W, Printmodul) mit Feinsicherung T 500 mA und Varistor S07K275 (siehe „Versorgung") | [Reichelt IRM-10-5](https://www.reichelt.de/de/de/shop/search/irm-10-5) | 8,10 € (2026-10-24) |
| 3,3-V-Regler | LDO AP7361C-33 (1 A, SOT-223) mit Kupferfläche | offen | offen |
| Programmierung | USB-C direkt am nativen USB des ESP32-S3 (kein USB-UART-Chip), ESD-Schutz, VBUS über Schottky-Diode, Taster BOOT und RESET | offen | offen |
| Anzeige | 2,8" TFT ST7789, 240 × 320 hochkant, SPI (festgelegt in Feature 001). Modul ohne eigenen Regler: alle Pins 3,3 V | vorhanden | – |
| Eingabe: Drehencoder | Alps EC11E15244G1: 15 Impulse / 30 Rastungen, Taster, vertikal, 20 mm Welle (siehe „Bedienfeld") | [Conrad 3523314](https://www.conrad.de/de/p/alps-encoder-1-st-5vdc-encoder-inkremental-pos-30-15impulse-umdrehung-10ma-tht-3523314.html) | 3,49 € (2026-10-24) |
| Eingabe: Stopp-Taste | rot, Ø 16–19 mm, Fronteinbau, **1 Öffner + 1 Schließer** (siehe „Bedienfeld" und „Lastseite") | offen | offen |
| Signal | Buzzer | offen | offen |
| Debug-LED | LED mit Vorwiderstand auf der Platine, nur zur Diagnose (nicht im Bedienfeld) | offen | offen |
| Gehäuse | auf der kalten Seite des Ofens | – | – |

Die Pinbelegung des Steckbrett-Prototyps („PCB REV DEV") steht in
`specs/001-startup-status-menu/research.md` (R13). Der Entwurf für die Platine (REV A) steht
unten unter „Mikrocontroller und Pinbelegung".

## Bedienfeld (Platz für Bedienelemente)

Gemessen an der alten, zu ersetzenden Bedieneinheit (Stand 2026-10-24). Das ist der nutzbare
Platz für Bedienelemente, **nicht** die Außenmaße des späteren CAD-Modells.

| Maß | Wert |
|---|---|
| Höhe | 175 mm |
| Breite oben und unten | 52 mm |
| Breite Mitte | 56 mm |
| Form | links gerade, rechts nach außen gewölbt (folgt der Ofenrundung) |

Vorhandenes Display: 2,8" TFT auf Trägerplatine **50 × 81,6 mm**. Es passt nur im
Hochformat (240 × 320 px) und lässt in der Breite je Seite rund 1 mm Rand. Unter dem Display
bleiben etwa 93 mm Höhe für weitere Bedienelemente.

### Entscheidung Bedienkonzept (2026-10-24)

Bedienelemente: **TFT-Display, Drehencoder mit Taster, separate Stopp-Taste.** Kein Touch
(zu fummelig), kein Ziffernfeld (passt nicht), keine Status-LED (das Display zeigt alles).

| Element | Belegung |
|---|---|
| Encoder drehen | Eintrag wählen, Wert ändern (mit Beschleunigung) |
| Encoder kurz drücken | OK / Auswahl bestätigen |
| Encoder lang drücken (ca. 1 s) | Zurück, eine Ebene höher; zusätzlich Menüeintrag „‹ Zurück" |
| Stopp-Taste (rot, Ø 16–19 mm) | Heizen sofort abbrechen; im Leerlauf zurück ins Hauptmenü. Startet nie etwas |

- Heizen wird nur über das Menü mit Bestätigung („Starten?") gestartet; keine kombinierte
  Start/Stopp-Taste.
- Die Stopp-Taste unterbricht zusätzlich zum Firmware-Eingang die SSR-Ansteuerung
  hardwareseitig (Niederspannungsseite): Ihr Öffner löscht den Freigabe-Latch, der Schließer
  geht an einen GPIO. Nach dem Loslassen bleibt die Heizung aus, bis die Firmware nach einer
  Benutzeraktion neu freigibt (siehe „Lastseite und Freigabekette").
- Buzzer kommt später auf die Platine bzw. hinter die Front (nur Schallöffnung).

### Entscheidung Drehencoder (2026-10-24)

- **Alps EC11E15244G1** (15 Impulse / 30 Rastungen, Taster mit 0,5 mm Hub, vertikal, 20 mm
  Welle, laut Alps 15.000 Zyklen). Das KY-040-Modul dient nur dem Steckbrett-Prototyp
  (No-Name-Encoder, starkes Prellen, kurze Welle, keine Temperaturangabe).
- **Fronteinbau:** Der Encoder sitzt mit Mutter in der Front, auf einem kleinen Träger
  (ca. 20 × 20 mm, Encoder + JST-Buchse) oder per Litze. So hängt die Lage der Hauptplatine
  nicht am Bedienfeld.
- Encoder, Stopp-Taste und ggf. Buzzer gehen über **einen gemeinsamen Steckverbinder** zur
  Hauptplatine. Kabel kurz, mit GND, auf der kalten Seite verlegt, Zugentlastung am Gehäuse.
- Pull-ups, RC-Entprellung (10 kΩ / 10 nF nach Datenblatt) und ESD-Schutz sitzen auf der
  Hauptplatine direkt am Steckverbinder.
- 15/30 bedeutet einen halben Quadraturzyklus pro Rastung; die Ruhelage wechselt zwischen
  offen und geschlossen. Der KY-040 liefert einen vollen Zyklus pro Rastung. Der Teiler
  (Schritte pro Rastung: 2 bzw. 4) gehört deshalb als Konstante in die Board-Konfiguration
  je PCB-Revision (`board_rev_*.h`), nicht fest in die HAL.
- Vor dem Einbau prüfen: Betriebstemperatur laut Datenblatt gegen die Temperatur an der Front.

Aufteilung der 175 mm Höhe (Vorschlag, von oben):

| Element | Höhe |
|---|---|
| Rand | ca. 5 mm |
| Display hochkant | 81,6 mm |
| Abstand | ca. 8 mm |
| Encoder, Knopf Ø 30 mm | ca. 40 mm |
| Abstand | ca. 8 mm |
| Stopp-Taste | ca. 22 mm |
| Reserve | ca. 10 mm |

## Lastseite und Freigabekette

### Entscheidung (2026-10-24)

| Punkt | Entscheidung |
|---|---|
| Heizung schalten | 2 Panel-SSRs (Markenware), nicht auf der Platine. Auf der Platine liegt nur Kleinspannung zur SSR-Ansteuerung |
| Sicherheitsrelais | ja, ein Relais in der gemeinsamen Phase vor beiden SSRs: Finder 40.61 in Fassung 95.05, angeschraubt. Hutschiene passt nicht |
| Externer Watchdog | ja, TPS3828-33 in der Löschkette |
| Stopp-Taste | 1 Öffner (löscht den Latch) + 1 Schließer (GPIO) |

### Begründung

- Last: oben und unten je ca. 600 W bzw. 2,6 A (beim Öffnen über den Widerstand der Heizstäbe
  nachmessen). 25-A-SSRs haben bewusst große Reserve. Verlustleistung ca. 1,3 V × 2,6 A ≈ 3,5 W
  je SSR: kleiner Kühlkörper oder Blech auf der kalten Seite mit Wärmeleitpad.
- Billige Marktplatz-SSRs („Fotek") sind oft Fälschungen mit zu kleinem Triac.
- **SSRs fallen meist kurzgeschlossen aus.** Dann heizt der Ofen voll durch, und weder Firmware
  noch Stopp-Taste könnten ihn abschalten. Deshalb trennt ein mechanisches Relais die
  Heizungszuleitung. Rollen: SSR = schnelles Takten, Relais = sichere Trennung bei Stopp, Fehler
  oder Übertemperatur, Thermosicherung = letzte Instanz.
- Die Spule des Relais (5 V, ca. 80–110 mA) treibt ein Transistor hinter dem Freigabe-Latch, mit
  Freilaufdiode (auf der Platine oder Steckmodul Finder 99.02).

### Schaltungsprinzip

Jedes SSR braucht zwei Bedingungen gleichzeitig, physisch in Reihe:

```
 +5V ──[P-MOSFET ← Latch Q]──┬─────────────────┬──── (+ Relaisspule über Transistor)
                             │                 │
                       SSR oben +        SSR unten +
                       SSR oben −        SSR unten −
                             │                 │
          GPIO HEAT_TOP ─[N-MOSFET]   GPIO HEAT_BOT ─[N-MOSFET]
          (Pull-down)        │        (Pull-down)      │
                            GND                       GND
```

- Low-Side je Kanal: Die Firmware taktet hier. Pull-downs halten im Reset, beim Booten und beim
  Flashen alles aus. Keine Strapping-Pins verwenden; Belegung siehe „Mikrocontroller und
  Pinbelegung".
- High-Side gemeinsam: SSR-Versorgung und Relais nur bei gesetztem Freigabe-Latch.

### Freigabe-Latch

D-Flipflop mit asynchronem Löschen (z. B. 74LVC1G74). Löschen hat Vorrang vor Setzen. Alle
Löschquellen sind Open-Drain bzw. Kontakte und werden auf der CLR-Leitung zusammengeschaltet
(Pull-up).

| Signal | Quelle | Wirkung |
|---|---|---|
| Löschen | Öffner der Stopp-Taste | Heizung sofort aus, ohne Firmware. Ein abgerissenes Kabel wirkt wie ein Stopp |
| Löschen | PCB-Übertemperatur (TC6501, Open-Drain) | erfüllt die SOLLTE-Regel aus Prinzip IVa |
| Löschen | FAULT der beiden MAX31856 (Open-Drain) | Kammer-Übertemperatur über eingestellte Schwelle oder Fühlerfehler, auch bei hängender Regelungslogik |
| Löschen | TPS3828 /RESET | Power-on-Reset, Unterspannung der 3,3-V-Schiene, Watchdog-Ablauf |
| Löschen | MCU „DISARM" über /MR des TPS3828 | Fehlerzustand der Firmware nimmt die Hardware-Freigabe weg |
| Setzen | MCU-Impuls „ARM" | nur nach bestätigtem „Starten?" |
| Rücklesen | Latch-Ausgang Q und Schließer der Stopp-Taste an je einen GPIO | Erkennung, Plausibilitätsprüfung, HIL-Test |

Nach jedem Löschen ist ein neues „ARM" und damit eine Benutzeraktion nötig. Solange Stopp
gedrückt ist, die Platine zu heiß ist oder der Watchdog auslöst, kann die Firmware nicht
freigeben.

### Externer Watchdog TPS3828-33

- SOT-23-5, Watchdog-Zeit 1,6 s, Reset-Puls 200 ms, Open-Drain-Ausgang, Eingang /MR, überwacht
  die 3,3-V-Schiene. Beschaltung: 100 nF an VDD, Pull-up an der CLR-Leitung.
- Die Firmware schaltet WDI nur aus der Hauptschleife um, und nur wenn die
  Sicherheitsüberwachung im letzten Zyklus rechtzeitig gelaufen ist. Nicht aus einem
  Timer-Interrupt.
- Während Boot und Flashen wird WDI nicht umgeschaltet; der Latch bleibt gelöscht.
- Offen: Prüfen, ob ein hochohmiger WDI den Watchdog abschaltet (TI-Applikationsschrift SLVA145).
  Falls ja, Pull-down an WDI vorsehen.

## Sensorik

### Entscheidung (2026-10-24)

| Punkt | Entscheidung |
|---|---|
| Kammertemperatur | 2 Kanäle: ein Fühler oben, einer unten. Je MAX31856 + Mini-TC-Buchse Typ K |
| Thermoelemente | Typ K, Mantel Ø 1–1,5 mm, isolierte Messstelle |
| PCB-Temperatur (Firmware) | CJ-Temperatur der MAX31856, kein eigener Sensor |
| PCB-Übertemperatur (Hardware) | TC6501, Schwelle ca. 75 °C, Hysterese 10 °C, am Hotspot |
| Fühler an der Lötplatine (Profilieren) | nicht vorgesehen |

Die Grenzwerte (Firmware-Grenze der PCB-Temperatur ca. 60–65 °C, Hardware-Schwelle ca. 75 °C,
FAULT-Schwelle der Kammer z. B. 280 °C, zulässige Abweichung oben/unten) werden in der
Spezifikation festgelegt; vorher die Temperaturgrenzen des IRM-10-5 im Datenblatt prüfen.

### Begründung

- **MAX31856 statt MAX31855:** umschaltbarer 50/60-Hz-Filter (Thermoelement liegt neben
  getakteten 230-V-Leitungen), einstellbare Temperaturschwellen und FAULT-Ausgang (Open-Drain)
  auf der CLR-Leitung des Freigabe-Latch. Die Schwellen schreibt die Firmware nach dem Start
  in die Register und liest sie zyklisch zurück; bis dahin ist der Latch gelöscht. Der
  MAX31855-Treiber bleibt für REV DEV; für die Platine kommt ein eigener HAL-Treiber
  (SPI-Modus 1/3, lesen und schreiben). `ITemperatureSensor` bleibt unverändert, je Kanal eine
  Instanz.
- **Zwei Fühler:** Plausibilitätsprüfung gegen einen verrutschten, abgelösten oder driftenden
  Fühler, der zu wenig anzeigt (Thermal Runaway). Später Ausgleich zwischen Ober- und
  Unterhitze über die beiden SSRs. Weil oben und unten im Normalbetrieb unterschiedlich warm
  sind, braucht die Plausibilitätsprüfung eine zulässige Abweichung.
- **Isolierte Messstelle:** Eine geerdete Messstelle verbindet das Thermoelement über den Mantel
  mit dem Ofengehäuse (PE). Das stört die Messung und erzeugt Fehlermeldungen beim MAX318xx.
- **Buchse neben dem IC:** Der Übergang Thermoelement-Material → Kupfer muss dieselbe Temperatur
  haben wie der IC (Vergleichsstelle). Buchsen und ICs auf einer gemeinsamen, ruhigen
  Kupferfläche an der Seite zur Heizkammer, weit weg von Netzteil und LDO. Am Eingang 10 nF
  differenziell, optional Ferrite nach Datenblatt.
- **CJ als PCB-Temperatur:** SSRs und Relais sitzen außerhalb; auf der Platine sind nur
  Netzteil (ca. 0,5 W) und LDO (ca. 0,45 W) kleine Wärmequellen. Die kritische Größe ist damit
  die Umgebungstemperatur im Elektronikfach, und die messen die MAX31856 an der Kammerseite.
  Diese Begründung für Prinzip IVa („thermisch kritischste Stelle") gehört in den Plan. Fällt
  ein MAX31856 aus, fehlen Kammer- und PCB-Wert dieses Kanals; beides führt zu „Heizung aus".
- **TC6501:** Temperaturschalter mit ab Werk fester Schwelle (+35 bis +125 °C in 10-°C-Schritten,
  Auswahl über die Bestellnummer, z. B. TC6501P075…). Hysterese per Pin 2 °C (GND) oder 10 °C
  (VCC), Pin nicht offen lassen. ±0,5 °C typisch, laut Tabelle im ungünstigsten Fall ±4–6 °C.
  Wirkt ohne Firmware. Sitzt am Hotspot zwischen Netzteil und LDO. Nach dem Abkühlen bleibt der
  Latch gelöscht; Heizen braucht ein neues „Starten?". Verfügbarkeit der gewählten Schwelle bei
  der Bestellung prüfen.
- Pro Kanal eine zusätzliche GPIO als Chip-Select; SPI gemeinsam, FAULT-Ausgänge gemeinsam auf
  der CLR-Leitung.

## Versorgung

### Entscheidung (2026-10-24)

| Punkt | Entscheidung |
|---|---|
| Netzteil | Mean Well IRM-10-5 statt HLK-5M05 |
| 3,3 V | LDO AP7361C-33 statt Schaltregler |
| Programmierung | USB-C auf der Platine am nativen USB des ESP32-S3 (ursprünglich USB-UART-Chip; entfällt mit dem Wechsel auf den S3) |
| Display | wird aus 3,3 V versorgt (Modul hat keinen Regler) |

### Strombudget

| Verbraucher | Schiene | typisch | Spitze | Anmerkung |
|---|---|---|---|---|
| ESP32-S3-WROOM-1 mit WLAN | 3,3 V | 100–150 mA | ca. 500 mA | Sendespitzen von wenigen ms |
| TFT-Logik + Hintergrundbeleuchtung | 3,3 V | 50–100 mA | 100 mA | Strom der Beleuchtung am Modul nachmessen |
| 2 × MAX31856, TPS3828, Logik, Debug-LED | 3,3 V | ca. 20 mA | – | |
| Sicherheitsrelais Finder 40.61, 5 V | 5 V | ca. 130 mA | – | aus 6-V-Variante abgeleitet (55 Ω / 109 mA ≈ 0,65 W), beim Kauf prüfen |
| 2 × SSR-Eingang | 5 V | 20–30 mA | 30 mA | |
| Buzzer | 5 V | – | 30 mA | |
| **Summe 5 V** | | **ca. 450 mA** | **ca. 800 mA** | |

### Begründung

- **IRM-10-5:** zertifizierte, dokumentierte Isolation (Prinzip IVb). 10 W lassen Reserve für
  die Leistungsreduzierung bei warmer Umgebung im Elektronikfach. Hi-Link-Module haben bekannte
  Probleme mit Zertifikaten und Fälschungen.
- **Eingangsseite:** Feinsicherung T 500 mA, dahinter Varistor S07K275 zwischen L und N.
  Sperrzone bzw. Schlitz unter dem Modul. Das ist der einzige 230-V-Bereich auf der Platine.
- **LDO statt Schaltregler:** Keine Schaltstörungen an der µV-Messung des Thermoelements.
  Verlustleistung im Mittel ca. (5 − 3,3) V × 0,27 A ≈ 0,45 W, bei WLAN-Spitzen kurz ca. 1 W.
  Der LDO ist damit eine Wärmequelle auf der Platine; seine Lage gegenüber dem
  PCB-Temperatursensor im Layout bewusst festlegen. Pufferkondensatoren am ESP32
  (10 µF + 100 µF).
- **Display:** Das Modul braucht 3,3 V an allen Pins und belastet damit den LDO. Die
  Hintergrundbeleuchtung über einen Transistor (P-MOSFET auf der High-Side, Gate mit Pull-up)
  per PWM schalten: dimmbar und während des Bootens dunkel.

### Ansteuerung der Relaisspule

```
 +5V ──┬──────────────┐
       │              │
  [Freilaufdiode]  Relaisspule (ca. 38 Ω, 130 mA)
       │              │
       └──────────────┤
                      │
   Latch Q ──[100 Ω]──┤ N-MOSFET (Logic-Level, z. B. AO3400)
            [100 kΩ Pull-down]
                      │
                     GND
```

- Pull-down am Gate: Relais im Reset sicher aus. Fällt die 5-V-Schiene aus, fällt das Relais
  ab (Schließer).
- Freilaufdiode (1N4148 oder SS14) verzögert das Abfallen um einige ms; unkritisch. Für
  schnelleres Abfallen Diode + Z-Diode.
- Normalbetrieb: Firmware schaltet erst die SSRs aus, dann das Relais (stromlos schalten,
  weniger Verschleiß). Bei Stopp oder Fehler trennen beide gleichzeitig; 5 A ohmsch sind für den
  16-A-Kontakt unkritisch.

### USB

- USB-C-Buchse mit 5,1-kΩ-Widerständen an CC1/CC2, ESD-Schutz (z. B. USBLC6-2) an D+/D−.
- D−/D+ direkt an GPIO 19/20 des ESP32-S3 (USB-Serial/JTAG im Chip): Flashen, serielles Log
  und Debugging ohne USB-UART-Chip und ohne Auto-Reset-Schaltung.
- Taster BOOT (GPIO 0) und RESET (EN) auf der Platine, um eine Firmware zu überschreiben, die
  das USB blockiert.
- VBUS über eine Schottky-Diode in die 5-V-Schiene, damit der PC nicht ins Netzteil
  zurückspeist. Flashen ohne Netz ist so möglich; Relais und SSRs schalten dabei nichts.

## Mikrocontroller und Pinbelegung

### Entscheidung (2026-10-24)

- **ESP32-S3-WROOM-1-N8** statt ESP32-WROOM-32E. Grund: Mit dem WROOM-32E waren alle nutzbaren
  GPIOs belegt, ohne Reserve. Der S3 hat deutlich mehr GPIOs und natives USB; USB-UART-Chip und
  Auto-Reset entfallen.
- Modulvariante **ohne Octal-PSRAM** (N8, N16 oder N8R2 mit Quad-PSRAM). Varianten mit
  Octal-PSRAM (z. B. N16R8) belegen GPIO 35–37.
- Prototyp vor der Platine: ESP32-S3-DevKitC-1-N8. Bis es da ist, bleibt REV DEV auf dem
  ESP32-DevKitC V4. Der Wechsel betrifft laut Verfassung nur HAL, Board-Konfiguration und
  PlatformIO-Umgebung; er wird im Plan des Features „Controller-Platine" festgehalten
  (Feature 001, research.md R1 nennt noch den ESP32).
- **Kein Lüfter**, dafür eine **Debug-LED** auf der Platine.

### Pins, die frei bleiben

| GPIO | Grund |
|---|---|
| 0, 3, 45, 46 | Strapping-Pins (Boot-Modus, JTAG-Quelle, Flash-Spannung). GPIO 0 nur für den BOOT-Taster |
| 19, 20 | natives USB |
| 26–32 | SPI-Flash des Moduls |
| 43, 44 | UART0: Boot-Meldungen, Reserve als Debug-Pinleiste |
| 38, 48 | auf dem DevKitC-1 an der RGB-LED (WS2812, je nach Board-Version); Reserve |
| 36, 37 | Reserve |

### Belegung REV A (Entwurf)

| Signal | GPIO | Bemerkung |
|---|---|---|
| Revisionserkennung (ADC) | 1 | ADC1; Spannungsteiler je Revision, einmal beim Start gelesen |
| Debug-LED | 2 | aktiv HIGH, mit Vorwiderstand |
| **HEAT_TOP** | 4 | Low-Side-MOSFET, externer Pull-down |
| **HEAT_BOT** | 5 | Low-Side-MOSFET, externer Pull-down |
| **ARM** (Latch setzen) | 6 | externer Pull-down; ein Glitch könnte den Latch setzen, deshalb kein Strapping- oder JTAG-Pin |
| TFT Hintergrundbeleuchtung | 7 | PWM, P-MOSFET mit Pull-up am Gate (beim Booten dunkel) |
| TFT RST | 8 | |
| TFT DC | 9 | |
| TFT CS | 10 | SPI2 über IO-MUX, Pull-up |
| TFT MOSI | 11 | SPI2 über IO-MUX |
| TFT SCK | 12 | SPI2 über IO-MUX (volle Taktrate fürs Display) |
| MAX31856 MISO | 13 | SPI3 über GPIO-Matrix |
| MAX31856 SCK | 14 | SPI3 |
| MAX31856 MOSI | 15 | SPI3 |
| MAX31856 CS oben | 16 | Pull-up |
| MAX31856 CS unten | 17 | Pull-up |
| DISARM (→ /MR des TPS3828) | 18 | ein Glitch löscht höchstens (sichere Richtung) |
| WDI (TPS3828) | 21 | |
| Buzzer | 35 | Treiber mit Pull-down |
| Encoder A | 39 | externer Pull-up + RC, Interrupt |
| Encoder B | 40 | externer Pull-up + RC, Interrupt |
| Encoder Taster | 41 | externer Pull-up + RC |
| Stopp-Schließer (Rücklesen) | 42 | externer Pull-up |
| Latch Q (Rücklesen) | 47 | |

- Display und Thermoelemente auf getrennten SPI-Bussen (wie in REV DEV): Eine Display-Übertragung
  kann keine Messung verzögern.
- Sicherheitsausgänge (HEAT_TOP, HEAT_BOT, ARM) liegen auf einfachen GPIOs mit externem Pull-down;
  GPIO 39–42 (JTAG-Pins) nur für Eingänge. Den Zustand aller Pins während Reset und Boot vor dem
  Layout gegen die Tabelle im Datenblatt des ESP32-S3 prüfen.
- Die Firmware zieht als erste Aktion nach dem Start DISARM, damit der Latch auch nach einem
  reinen MCU-Reset sofort gelöscht ist.
- Die Belegung kommt als `board_rev_a.h` in die Board-Konfiguration (Prinzip VII).

## Sicherheit der Hardware

- **Die Thermosicherung bleibt in Reihe.** Sie ist die letzte Abschaltung, unabhängig von der
  Firmware.
- Das Sicherheitsrelais trennt die Heizungszuleitung, wenn ein SSR kurzgeschlossen ausfällt
  (siehe „Lastseite und Freigabekette").
- Schutzleiter am Metallgehäuse anschließen und den Durchgang messen.
- Arbeiten an 230 V nur im stromlosen Zustand (Netzstecker gezogen).
- Nach dem Umbau ist der Ofen **kein Küchengerät** mehr (Flussmittel, evtl. bleihaltiges Lot).
  Nur mit Belüftung betreiben und deutlich kennzeichnen.

Die Sicherheitsfunktionen der Firmware (maximale Laufzeit, Übertemperatur, Fühlerfehler) werden im
Spec-Kit-Feature „Sichere Temperaturregelung“ beschrieben.
