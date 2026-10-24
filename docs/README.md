# Projekt-Dokumentation

Hintergrundwissen zum Reflow-/Silicagel-Ofen, das nicht über Spec Kit (`specs/`) verwaltet wird:
Umbauarbeiten am Ofen, Recherchen, Werkzeugentscheidungen und Anwendungshinweise.

Stand: 2026-10-24. Maße und Daten von Webseiten sind Herstellerangaben und nicht selbst
nachgemessen.

## Ziel des Projekts

Ein kleiner Ofen für zwei Zwecke:

1. **Reflow-Löten** zweiseitiger SMD-Platinen (typisch bis 10 × 10 cm, Auslegung auf 15 × 15 cm).
2. **Silicagel regenerieren** (ca. 100–110 °C, 1,5–3 h) für die Filamenttrocknung beim 3D-Druck.

Dafür wird ein günstiger Mini-Backofen mit einem eigenen ESP32-Controller (Platine: ESP32-S3)
umgebaut. Das
Controleo3-Kit ist eine teure Option, wurde verworfen um dem eigenen Basteltrieb zu fröhnen.

## Dokumente

| Datei | Inhalt |
|---|---|
| [hardware-umbau.md](hardware-umbau.md) | Ofen (Steinborg SB-3005), Dämmung, Verkabelung, Komponenten-Vorauswahl, Sicherheit der Hardware |
| [umluft-recherche.md](umluft-recherche.md) | Nachrüstung eines Umluftlüfters: Problem, Kandidaten, Ansteuerung |
| [werkzeuge-pcb.md](werkzeuge-pcb.md) | Werkzeugkette für Schaltplan und Platine (KiCad, Hardware-as-Code, MCP) |
| [anwendung.md](anwendung.md) | Praxishinweise zum Reflow-Löten und zum Regenerieren von Silicagel |
| [error-codes.md](error-codes.md) | Fehlercodes der Firmware mit Ursache und Abhilfe |
| [state-machine.md](state-machine.md) | Zustandsmaschinen der Firmware (Diagramm und Übergangstabelle) |
| [compatibility.md](compatibility.md) | Kompatibilität Firmware ↔ PCB ↔ Gehäuse, HIL- und Abnahmeergebnisse |

## Abgrenzung zu Spec Kit

Diese Funktionen werden als eigene Features mit `/speckit-specify` beschrieben, nicht hier:

1. Sichere Temperaturregelung (inkl. Sicherheitsabschaltungen)
2. Bedienung und Anzeige
3. Reflow-Programm
4. Silicagel-Programm
5. Profilverwaltung (optional)
6. Controller-Platine
7. Controller-Gehäuse
8. ~~Umluft-Ansteuerung~~ (entfällt, kein Lüfter; Entscheidung 2026-10-24)

Die Dokumente hier liefern dafür den Kontext und die Randbedingungen.

## Offene Punkte

- [ ] Ofen öffnen: Heizstäbe, Verdrahtung, Thermostat, Thermosicherung und Platz hinter der
      Rückwand dokumentieren und messen → Ergebnisse in [hardware-umbau.md](hardware-umbau.md)
- [x] Umluft ja/nein: **nein** (2026-10-24) → [umluft-recherche.md](umluft-recherche.md)
- [ ] Werkzeuge festlegen: atopile oder SKiDL, KiCad 9 oder 10, MCP ja/nein →
      [werkzeuge-pcb.md](werkzeuge-pcb.md)
- [ ] Einkaufsliste für Dämmung und Verkabelung erstellen (grob 120–200 € gesamt, je nach
      Bastelkiste)
