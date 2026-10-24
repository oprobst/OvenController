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
| Mikrocontroller | ESP32 |
| Temperaturmessung | MAX31855 (Thermoelement-Verstärker) mit dünnem Typ-K-Mantelthermoelement, Ø 1–1,5 mm |
| Heizung schalten | 2 × SSR, nullspannungsschaltend, 25 A, Markenware, mit Kühlkörper. Getrennt für Ober- und Unterhitze |
| Lüfter schalten (optional) | 3. SSR oder Relais |
| Netzteil | 230 V → 5 V, z. B. HLK-5M05 |
| Anzeige | OLED oder kleines TFT |
| Eingabe | Drehencoder |
| Signal | Buzzer |
| Gehäuse | auf der kalten Seite des Ofens |

## Sicherheit der Hardware

- **Die Thermosicherung bleibt in Reihe.** Sie ist die letzte Abschaltung, unabhängig von der
  Firmware.
- Schutzleiter am Metallgehäuse und am Lüftermotor anschließen und den Durchgang messen.
- Arbeiten an 230 V nur im stromlosen Zustand (Netzstecker gezogen).
- Nach dem Umbau ist der Ofen **kein Küchengerät** mehr (Flussmittel, evtl. bleihaltiges Lot).
  Nur mit Belüftung betreiben und deutlich kennzeichnen.

Die Sicherheitsfunktionen der Firmware (maximale Laufzeit, Übertemperatur, Fühlerfehler) werden im
Spec-Kit-Feature „Sichere Temperaturregelung“ beschrieben.
