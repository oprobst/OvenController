# Umluft – Recherche zur Nachrüstung

Status: **entschieden (2026-10-24): kein Lüfter.** Die Controller-Platine sieht keinen
Lüfterausgang vor. Die Recherche bleibt als Hintergrund erhalten.

## Warum Umluft

Umluft verteilt die Temperatur gleichmäßiger und beschleunigt das Trocknen von Silicagel. Zwingend
nötig ist sie nicht.

## Problem

Fast alle Backofen-Ersatzteile haben Flügel mit **150–180 mm**. Die Rückwand ist innen nur ca.
17,5 cm hoch. Gesucht ist ein Lüfterrad von etwa **110–120 mm**.

## Kandidaten

| Modell | Quelle / Preis | Daten | Bewertung |
|---|---|---|---|
| Amica 8512643 / KELI YJ61-16A | ersatzteilshop.de ca. 39 €, eBay ca. 35 € | 22 W, Motor ca. 79 × 66 × 83 mm (ohne Flügel), Flügel 152 × 152 × 20 mm | Maße bekannt. Flügel kürzen und auswuchten oder durch ein Airfryer-Ersatzrad (110–130 mm) ersetzen |
| daniplus Heißluftherdventilator (Bosch/Neff 096825) | Amazon | 22 W, 2.530 U/min, Flügel Ø 150 mm, Befestigungsabstand 90 mm, Achslänge ab Befestigung 40 mm | Ein Rezensent nennt leichte Unwucht |
| Galanz-Lüftermotor 278009000159 | ersatzteilshop.de ca. 18,89 € | Maße nicht angegeben | Vermutlich kompakter. **Maße beim Händler erfragen** |
| Beko C00866950 „kleines Lüfterrad“ | 7,99 € | Nur das Rad. Durchmesser und Wellenaufnahme unbekannt | Klären, ob es auf einen der Motoren passt |
| ROOwarMer 45 W | Amazon, ca. 29 € | Keine Radmaße angegeben | Zu stark |

**Nicht geeignet:** PC- und 12-V-Lüfter. Sie bestehen aus Kunststoff und halten nur ca. 70 °C aus.

## Ansteuerung

- Spaltpolmotor 230 V AC, geschaltet über SSR oder Relais.
- Drosseln (damit 0402-Bauteile nicht verrutschen) per Reihenkondensator (ca. 1–2 µF, X2) oder
  Phasenanschnitt.
- Temperaturangaben des Motors beachten.

## Nächste Schritte

- [ ] Platz hinter der Rückwand messen (siehe [hardware-umbau.md](hardware-umbau.md)).
- [ ] Maße des Galanz-Motors beim Händler erfragen.
- [ ] Entscheidung: Umluft ja/nein, welcher Motor, welches Rad.
