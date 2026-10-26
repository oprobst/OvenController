# Werkzeuge für Schaltplan und Platine

Status: **entschieden (2026-10-25), geändert am selben Tag**:

- **Quelle des Schaltplans ist der KiCad-10-Schaltplan** (`hardware/<platine>/<platine>.kicad_sch`).
  Layout, ERC/DRC und Exporte in **KiCad 10.0.3** (`D:\hobby\Elektronik\KiCad\10.0`,
  `kicad-cli` in `bin`). Kein MCP-Server.
- Der Schaltplan wurde zuerst als Code mit **SKiDL** geschrieben und geprüft (Feature 002, T010–T018).
  Weil der aus SKiDL erzeugte Schaltplan kaum lesbar war, ist er seitdem nur noch die
  **Vorlage**: einmal generiert, in KiCad aufgeräumt, ab dann gepflegt. Der SKiDL-Code liegt in
  `hardware/archive/skidl/` und wird nicht mehr geändert.
- Die damals geprüfte Netzliste ist als Referenz eingefroren (`<platine>.ref.net`).
  `hardware/tools/check_schematic.py` vergleicht den Schaltplan mit ihr (Bauteile, Verbindungen,
  Netznamen am ESP32) und prüft die GPIO-Belegung gegen `firmware/include/board/board_rev_a.h`.
  Eine gewollte Schaltungsänderung heißt: Referenznetzliste im selben Commit aktualisieren und
  den Grund nennen.

Warum nicht atopile: nur mit KiCad 9 erprobt und schreibt mit eigenem Parser in die
`.kicad_pcb` (Gefahr von Datenverlust beim Speichern in KiCad 10).

## Möglichkeiten, mit Claude an KiCad-Projekten zu arbeiten

### Ohne MCP

- `.kicad_sch` und `.kicad_pcb` sind Textdateien (S-Expressions). Claude kann sie lesen und
  bearbeiten.
- Prüfen und exportieren über `kicad-cli`: ERC, DRC, Gerber, BOM.
- Für komplette Schaltpläne ist das direkte Bearbeiten der Dateien fehleranfällig.

### MCP-Server (über die KiCad-IPC-API)

Bekannte Server: KiCad MCP Pro, kicad-mcp-kipy (KiCad 10), mixelpixx/KiCAD-MCP-Server.

Einschränkungen:

- funktioniert nur mit laufender KiCad-GUI
- kein Plot/Export (erst ab KiCad 11)
- Schaltplan-Bearbeitung in KiCad 10 nicht möglich (für KiCad 11 angekündigt)

Gut geeignet für Inspektion, Bearbeitung und Reviews der Platine.

### Hardware-as-Code (atopile, SKiDL, tscircuit)

Derzeit der stärkste Weg, mit einem LLM Schaltpläne zu erstellen. Der Code wird zu einem
KiCad-Projekt kompiliert und lässt sich wie Software versionieren und reviewen.

## Vorschlag

1. Schaltplan als Code (atopile oder SKiDL)
2. Layout in KiCad, ggf. mit Freerouting
3. Review und Export per `kicad-cli`
4. Optional ein MCP-Server für Live-Reviews im Layout

## Regel

Pinbelegungen und Footprints **immer gegen das Datenblatt prüfen**. Hier machen KI-Werkzeuge
typischerweise Fehler.

## Erkenntnisse aus dem Layout (KiCad 10.0.3)

- **Abstand Kupfer zu Bohrungen ohne Kupfer (NPTH)**: Eine eigene Regel mit
  `hole_clearance` wird für NPTH-Bohrungen nicht geprüft, auch nicht bei offensichtlicher
  Verletzung. `physical_hole_clearance` wirkt (beide Reihenfolgen von A und B). Getestet am
  2026-10-26 mit einer Kopie der Platine; die Regeln `mains_to_mounting_holes` und
  `cover_holes_to_mains` in `hardware/controller/controller.kicad_dru` nutzen deshalb
  `physical_hole_clearance`.
- **NPTH-Pads verlieren ihre Nummer** beim Laden: Pads einer Footprint-Bohrung lassen sich in
  Skripten nicht über `GetNumber()` finden, nur über die Position.
- **Rule Areas in Footprints** („keine Footprints“) melden Bauteile, deren Courtyard hineinragt
  (`items_not_allowed`); so zeigt der Footprint der Netzabdeckung (COV1) Bauteile unter ihrer Wand.
