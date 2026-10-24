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
| Anzahl und Anordnung der Heizstäbe (oben/unten getrennt?) | _offen_ |
| Position Thermostat | _offen_ |
| Position Timer, hat er eine Dauerstellung? | _offen_ |
| Position Thermosicherung | _offen_ |
| Freier Platz hinter der Rückwand (für Lüfter) | _offen_ |
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

| Funktion | Vorauswahl |
|---|---|
| Mikrocontroller | ESP32 (ESP32-WROOM-32); Prototyp auf ESP32-DevKitC V4 (festgelegt in Feature 001) |
| Temperaturmessung | MAX31855 (Thermoelement-Verstärker) mit dünnem Typ-K-Mantelthermoelement, Ø 1–1,5 mm |
| Heizung schalten | 2 × SSR, nullspannungsschaltend, 25 A, Markenware, mit Kühlkörper. Getrennt für Ober- und Unterhitze |
| Lüfter schalten (optional) | 3. SSR oder Relais |
| Netzteil | 230 V → 5 V, z. B. HLK-5M05 |
| Anzeige | 2,8" TFT ST7789, 240 × 320 hochkant, SPI (festgelegt in Feature 001) |
| Eingabe | Drehencoder mit Taster und separate Stopp-Taste (siehe „Bedienfeld") |
| Signal | Buzzer |
| Gehäuse | auf der kalten Seite des Ofens |

Die Pinbelegung des Steckbrett-Prototyps („PCB REV DEV") steht in
`specs/001-startup-status-menu/research.md` (R13).

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
- Die Stopp-Taste soll zusätzlich zum Firmware-Eingang die SSR-Ansteuerung hardwareseitig
  unterbrechen (Niederspannungsseite). Weil der Kontakt beim Loslassen wieder schließt, muss
  die Firmware den Druck erkennen und abschalten oder eine Selbsthaltung das Wiedereinschalten
  verhindern. Details im Feature „Controller-Platine".
- Buzzer kommt später auf die Platine bzw. hinter die Front (nur Schallöffnung).

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

## Sicherheit der Hardware

- **Die Thermosicherung bleibt in Reihe.** Sie ist die letzte Abschaltung, unabhängig von der
  Firmware.
- Schutzleiter am Metallgehäuse und am Lüftermotor anschließen und den Durchgang messen.
- Arbeiten an 230 V nur im stromlosen Zustand (Netzstecker gezogen).
- Nach dem Umbau ist der Ofen **kein Küchengerät** mehr (Flussmittel, evtl. bleihaltiges Lot).
  Nur mit Belüftung betreiben und deutlich kennzeichnen.

Die Sicherheitsfunktionen der Firmware (maximale Laufzeit, Übertemperatur, Fühlerfehler) werden im
Spec-Kit-Feature „Sichere Temperaturregelung“ beschrieben.
