# Werkzeuge für Schaltplan und Platine

Status: **entschieden (2026-10-25)**: Schaltplan als Code mit **SKiDL**, Layout und Prüfung in
**KiCad 10.0.3** (`D:\hobby\Elektronik\KiCad\10.0`, `kicad-cli` in `bin`). Kein MCP-Server
vorerst. Begründung und Risiken: `specs/002-controller-pcb/research.md` (R1).

Kurzfassung: atopile ist nur mit KiCad 9 erprobt und schreibt mit eigenem Parser in die
`.kicad_pcb` (Gefahr von Datenverlust beim Speichern in KiCad 10). SKiDL erzeugt nur eine
Netzliste; das Layout bleibt vollständig in KiCad.

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
