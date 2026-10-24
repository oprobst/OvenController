# Anwendungshinweise

Praxiswissen für den Betrieb des Ofens. Was die Firmware dafür automatisch tun soll, steht in den
Spec-Kit-Features „Reflow-Programm“ und „Silicagel-Programm“.

## Reflow-Löten

- **Zweiseitige Platinen:**
  - zuerst die Seite mit den leichten Bauteilen löten
  - schwere Teile auf der Unterseite kleben oder von Hand nachlöten
  - alternativ für die zweite Seite Niedertemperaturpaste verwenden (SnBi, ca. 138 °C)
- Unterhitze betonen, Oberhitze reduzieren (der Controller taktet die Oberhitze schwächer).
- Thermoelement nah an der Platine platzieren, nicht an der Ofenwand.
- Platine auf dem Alublech-Träger einlegen (siehe [hardware-umbau.md](hardware-umbau.md)).
- Zum Abkühlen nach dem Peak die Tür öffnen (später optional Lüfter oder Servo).

## Silicagel regenerieren

- Temperatur: ca. 105 °C halten, **maximal 120 °C** (Indikatorgel, Tyvek-Beutel).
- Dauer: 1,5–3 h nach einer Aufheizrampe von ca. 10 min.
- Metallschale (Alu oder Edelstahl) verwenden, **kein Kunststoff**.
- Schichthöhe maximal 1–2 cm.
- Tür einen Spalt offen lassen oder ein Entlüftungsloch vorsehen, damit der Wasserdampf entweicht.
- Das Gel anschließend noch warm in ein geschlossenes Glas füllen.
- Idee für später: Wägezelle, die das Programm automatisch beendet, wenn kein Gewicht mehr verloren
  geht.
