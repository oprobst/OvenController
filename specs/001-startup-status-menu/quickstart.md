# Quickstart: Startbildschirm mit Statusanzeige und Hauptmenü

**Feature**: [spec.md](spec.md) | **Plan**: [plan.md](plan.md)

Anleitung zum Aufbauen, Bauen, Testen und Abnehmen des Features.

## Voraussetzungen

- PlatformIO Core ≥ 6.1 (auf dem Entwicklungsrechner vorhanden: 6.1.19)
- Für `native`-Tests unter Windows: GCC für den Host, z. B. MSYS2 MinGW-w64
  (`pacman -S mingw-w64-ucrt-x86_64-gcc`, danach `C:\msys64\ucrt64\bin` in den `PATH`).
  Ohne lokalen GCC laufen die Tests nur in der CI. Alternative: PlatformIO in WSL (Ubuntu mit
  g++) installieren und `pio test -d firmware -e native` dort ausführen.
- Hardware (Steckbrett, nur 5 V über USB, **keine Netzspannung**):
  - ESP32-DevKitC V4
  - 2,8"-TFT ST7789 240×320 (SPI), hochkant eingebaut
  - MAX31855K-Breakout mit Typ-K-Thermoelement
  - Drehencoder-Modul mit Taster (z. B. KY-040)
- Verdrahtung nach [research.md R13](research.md#r13--pinbelegung-rev-dev-esp32-devkitc-v4).
  Vorher gegen die Datenblätter prüfen.

## WLAN-Zugangsdaten anlegen

```bash
cp firmware/include/wifi_secrets.example.h firmware/include/wifi_secrets.h
```

Netzwerkname und Passwort in `wifi_secrets.h` eintragen. Die Datei ist in `.gitignore` und
darf nicht committet werden. Ohne die Datei baut die Firmware trotzdem und zeigt
„WLAN nicht eingerichtet".

## Bauen und Flashen

```bash
pio run -d firmware -e esp32_revdev -t upload
```

```bash
pio device monitor -d firmware -e esp32_revdev
```

## Tests

Host-Tests (Logik, ohne Hardware):

```bash
pio test -d firmware -e native
```

HIL-Tests (Board angeschlossen, Thermoelement bei Raumtemperatur):

```bash
pio test -d firmware -e esp32_revdev --filter "hil/*"
```

Statische Analyse:

```bash
pio check -d firmware -e esp32_revdev
```

## Abnahme (manuell, entspricht der Spezifikation)

| # | Schritt | Erwartung | Bezug |
|---|---|---|---|
| 1 | USB einstecken, Zeit stoppen | Begrüßung mit „Firmware 0.1.0" und „PCB REV DEV" nach ≤ 2 s; Log-Zeile `msg="boot"` | US1-1, SC-001, FR-001 |
| 2 | Nichts tun | Nach 3 s Hauptmenü mit „Heizen", „Profil", „Setup", „Heizen" markiert | US2-1, FR-011 |
| 3 | Neustart (EN-Taste), sofort Encoder drücken | Sofortiger Wechsel ins Menü | US2-2, FR-002 |
| 4 | Encoder in beide Richtungen drehen | Markierung wandert, bleibt an Anfang/Ende stehen | US2-3 |
| 5 | Jeden Eintrag drücken | Keine Änderung auf dem Display; Log `msg="menu select" action=none` | US2-5 |
| 6 | Mit erreichbarem WLAN starten | „WLAN verbunden", nach ≤ 30 s Uhrzeit/Datum korrekt (± 1 min) | US1-2, SC-003 |
| 7 | Ohne `wifi_secrets.h` bauen und starten | „WLAN nicht eingerichtet", keine Uhrzeit; Menü nach ≤ 5 s bedienbar | Edge Case, SC-002 |
| 8 | Mit falschem Passwort starten | „WLAN nicht verbunden", keine Uhrzeit, Menü bedienbar | US1-3, FR-008 |
| 9 | Router ausschalten, während verbunden | Nach ≤ 10 s „WLAN nicht verbunden"; Uhrzeit läuft weiter | Edge Case |
| 10 | Temperatur mit Referenzthermometer vergleichen | Abweichung ≤ 3 °C; Wert ändert sich bei Erwärmung (Hand) innerhalb 1 s | US1-4, SC-004 |
| 11 | Thermoelement abziehen | ≤ 2 s „E101" in der Statuszeile und „Fühlerfehler E101"; Log `code=E101` | FR-007, SC-005 |
| 12 | Thermoelement wieder anstecken | Temperatur erscheint wieder, Fehlertext verschwindet | Edge Case |
| 13 | Serielles Log über 5 min beobachten | Keine Neustarts, keine `lvl=ERROR`-Zeilen | Prinzip IVa |

## Ergebnis dokumentieren

HIL- und Abnahmeergebnis (Datum, Firmware-Version, Board-Revision, bestanden/nicht bestanden)
in `docs/compatibility.md` eintragen (Prinzip III/VI).
