# Reflow-/Silicagel-Ofen

Umbau eines Mini-Backofens (Steinborg SB-3005) mit eigenem ESP32-Controller für Reflow-Löten und
das Regenerieren von Silicagel.

## Wo was steht

- `.specify/memory/constitution.md` – verbindliche Grundregeln des Projekts (Spec Kit)
- `specs/` – Feature-Spezifikationen, Pläne und Aufgaben (Spec Kit)
- `hardware/` – KiCad-10-Projekte; Quelle ist der KiCad-Schaltplan, Prüfung gegen die
  Referenznetzliste mit `hardware/tools/check_schematic.py` (siehe `docs/werkzeuge-pcb.md`).
  `kicad-cli` liegt unter `D:\hobby\Elektronik\KiCad\10.0\bin`
- `docs/` – Hardware-Wissen außerhalb von Spec Kit. Einstieg: `docs/README.md`
  - `docs/hardware-umbau.md` – Ofen-Maße, Dämmung, Verkabelung, Komponenten-Vorauswahl
  - `docs/umluft-recherche.md` – Lüfter-Nachrüstung
  - `docs/werkzeuge-pcb.md` – Werkzeugkette KiCad / Hardware-as-Code
  - `docs/anwendung.md` – Praxishinweise Reflow und Silicagel

Bei Fragen zur Hardware, zu Maßen, Bauteilen oder Werkzeugen zuerst in `docs/` nachsehen. Neue
Erkenntnisse (Messwerte, Entscheidungen) dort eintragen.
