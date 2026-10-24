# Contract: Bildschirme

**Feature**: [../spec.md](../spec.md) | **Display**: 240 × 320 px, Hochformat

Das Display sitzt hochkant im Bedienfeld (Trägerplatine 50 × 81,6 mm, siehe
`docs/hardware-umbau.md`). Alle Texte auf Deutsch. Farben sind logische Rollen (`Color`);
die HAL bildet sie auf RGB565 ab (dunkler Hintergrund, helle Schrift, Akzent für Markierung,
Gelb für Warnung, Rot für Fehler, Grau für Platzhalter).

## Statusbereich (auf beiden Bildschirmen, y = 0 … 47)

Zwei Zeilen à 24 px, darunter eine Trennlinie bei y = 48 (1 px, Muted).

```text
┌────────────────────────────┐
│ WLAN verbunden       14:32 │  Zeile 1, y = 0 … 23
│ 24.10.2026           23 °C │  Zeile 2, y = 24 … 47
├────────────────────────────┤
```

| Bereich | Position | Inhalt |
|---|---|---|
| WLAN | Zeile 1 links, 160 px, Small | „WLAN verbunden" (Foreground) · „WLAN nicht verbunden" (Warning) · „WLAN nicht eingerichtet" (Muted) |
| Uhrzeit | Zeile 1 rechts, 80 px, Medium, rechtsbündig | `HH:MM`; bei `Connected` ohne gültige Zeit `--:--` (Muted) |
| Datum | Zeile 2 links, 120 px, Small | `TT.MM.JJJJ`; bei `Connected` ohne gültige Zeit `--.--.----` (Muted) |
| Temperatur | Zeile 2 rechts, 120 px, Medium, rechtsbündig | `23 °C` (gerundet); vor erster Messung `--- °C` (Muted); bei Fehler `E101` (Error) |

Uhrzeit und Datum bleiben leer, wenn das WLAN nicht verbunden ist und die Zeit nie
synchronisiert wurde. Nach einmaliger Synchronisation wird immer die laufende Zeit angezeigt.

Bei einem Fühlerfehler erscheint zusätzlich unten im Inhaltsbereich (y = 292 … 319) der Text
„Fühlerfehler E101" (Error, zentriert), solange der Fehler besteht.

## Begrüßung (`UiScreen::Welcome`)

```text
┌────────────────────────────┐
│ <Statusbereich>            │
├────────────────────────────┤
│                            │
│        Willkommen!         │  Large, Center, y ≈ 110
│      Ofen-Controller       │  Medium, Center, y ≈ 150
│                            │
│       Firmware 0.1.0       │  Small, Muted, Center, y ≈ 220
│        PCB REV DEV         │  Small, Muted, Center, y ≈ 240
│                            │
└────────────────────────────┘
```

Dauer 3000 ms (`kWelcomeDuration`), vorzeitig per Encoder-Druck beendet.

## Hauptmenü (`UiScreen::MainMenu`)

```text
┌────────────────────────────┐
│ <Statusbereich>            │
├────────────────────────────┤
│  Hauptmenü                 │  Medium, Muted, y = 56 … 79
│ ▶ Heizen                   │  markiert: Accent-Hintergrund
│   Profil                   │
│   Setup                    │
│                            │
└────────────────────────────┘
```

- Einträge ab y = 84, Eintragshöhe 40 px, volle Breite, Schrift `Large`; bis zu 5 Einträge
  sichtbar (y = 84 … 283), darunter bleibt der Platz für die Fühlerfehler-Zeile frei.
- Markierter Eintrag: Hintergrund `Accent`, Text `Background`.
- Beim Drehen werden nur der alte und der neue markierte Eintrag neu gezeichnet.
- Drücken: keine sichtbare Änderung (Spezifikation US2, Szenario 5).

## Aktualisierung

- Der Statusbereich wird alle 100 ms geprüft; ein Teilbereich wird nur neu gezeichnet, wenn
  sich sein Text geändert hat.
- Beim Bildschirmwechsel wird der Inhaltsbereich (y = 49 … 319) einmal komplett gelöscht.
