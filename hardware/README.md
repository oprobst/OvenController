# Hardware

KiCad-Projekte des Ofen-Controllers. Entscheidungen und Begründungen:
`specs/002-controller-pcb/plan.md` und `research.md`, Werkzeugwahl: `docs/werkzeuge-pcb.md`.

| Verzeichnis | Inhalt |
|---|---|
| `lib/` | projekteigene Symbole (`oven.kicad_sym`) und Footprints (`oven.pretty/`), Prüfliste in `lib/README.md` |
| `controller/` | Hauptplatine REV A: Schaltplan (`controller.kicad_sch` + 2 Blätter: `controller_leistung` (A3: Netzteil, Sicherheit, Last) und `controller_steuerung` (A2: MCU, USB, Thermoelemente, Bedienung)), Platine, Designregeln, Referenznetzliste `controller.ref.net`, Exporte in `output/` |
| `encoder-carrier/` | Encoder-Träger REV A, gleiche Struktur |
| `tools/` | Prüf- und Hilfsskripte (siehe unten) |
| `archive/skidl/` | früherer Schaltplan als SKiDL-Code (nur noch Archiv) |

## Werkzeugkette

- **Quelle ist der KiCad-10-Schaltplan.** KiCad 10.0.3 unter `D:\hobby\Elektronik\KiCad\10.0`,
  Kommandozeile `D:\hobby\Elektronik\KiCad\10.0\bin\kicad-cli.exe`.
- Die Schaltpläne wurden einmalig aus dem SKiDL-Code (`archive/skidl/`) als Vorlage erzeugt; die
  damals geprüfte Netzliste ist als `<platine>.ref.net` eingefroren.
- Die Platine wird in KiCad aus dem Schaltplan aktualisiert (PCB-Editor: Werkzeuge → „Platine
  aus Schaltplan aktualisieren", F8).
- Die Skripte in `tools/` laufen mit normalem Python (`check_schematic.py`, `make_project.py`)
  oder mit KiCads Python (`D:/hobby/Elektronik/KiCad/10.0/bin/python.exe`, alle mit `pcbnew`).

## KiCad-Projekt anlegen (einmalig je Platine)

```bash
"D:/hobby/Elektronik/KiCad/10.0/bin/python.exe" hardware/tools/make_board.py hardware/controller/controller.kicad_pcb
```

```bash
python hardware/tools/make_project.py hardware/controller/controller.kicad_pro
```

Reihenfolge einhalten: Das Speichern einer neuen Platine setzt die Projektdatei zurück.
`make_project.py` setzt die Netzklassen `Mains` (L_IN, N_IN, L_FUSED), `Power` und `Default`
und die Mindestwerte. Die Sicherheitsabstände stehen als eigene Regeln in
`controller/controller.kicad_dru`: Netz ↔ Kleinspannung 6 mm (Luft- und Kriechstrecke), L ↔ N
3 mm, keine Vias in Netzleitungen, Netz ↔ Platinenrand 3 mm, Netz ↔ Montagelöcher 3 mm. Die
ersten vier Regeln sind an einer Testplatine mit absichtlichen Verstößen geprüft (T019), die
Regel zu den Montagelöchern wird mit dem ersten Layout geprüft. `controller/fp-lib-table` bindet
`hardware/lib/oven.pretty` für die KiCad-Oberfläche ein.

## Schaltplan prüfen (nach jeder Änderung)

```bash
python hardware/tools/check_schematic.py controller
```

```bash
python hardware/tools/check_schematic.py encoder-carrier
```

Exportiert die Netzliste des Schaltplans mit `kicad-cli` und vergleicht sie mit der
Referenznetzliste: Bauteile (Referenz, Wert, Footprint, MPN, DNP), Verbindungen (welche Pins
zusammenhängen, unabhängig vom Netznamen), Netznamen am ESP32 und an allen Netzen mit Netzklasse
(`Mains`, `Power` aus der `.kicad_pro`). Bei der Hauptplatine zusätzlich der GPIO-Abgleich mit
`firmware/include/board/board_rev_a.h`. Exit-Code 1 bei jeder Abweichung, umbenannte Netze ohne
Netzklasse sind nur Hinweise.

**Gewollte Schaltungsänderung:** Schaltplan ändern, ERC, dann die Referenznetzliste im selben
Commit neu exportieren und den Grund in der Commit-Nachricht nennen:

```bash
"D:/hobby/Elektronik/KiCad/10.0/bin/kicad-cli.exe" sch export netlist --output hardware/controller/controller.ref.net hardware/controller/controller.kicad_sch
```

ERC des Schaltplans:

```bash
"D:/hobby/Elektronik/KiCad/10.0/bin/kicad-cli.exe" sch erc --exit-code-violations --output hardware/controller/output/erc.rpt hardware/controller/controller.kicad_sch
```

## Schaltplan und Platine abgleichen

Normalerweise in KiCad mit F8. Skriptweise (z. B. nach dem Wechsel der Quelle):

```bash
"D:/hobby/Elektronik/KiCad/10.0/bin/kicad-cli.exe" sch export netlist --output hardware/controller/output/controller.net hardware/controller/controller.kicad_sch
```

```bash
"D:/hobby/Elektronik/KiCad/10.0/bin/python.exe" hardware/tools/import_netlist.py hardware/controller/output/controller.net hardware/controller/controller.kicad_pcb
```

Hat ein Symbol eine neue UUID bekommen (z. B. nach Ersetzen oder Einfügen in KiCad), verknüpft das
Skript den Footprint über die Referenz neu, statt ihn zu löschen; die Platzierung bleibt erhalten
und jede Neuverknüpfung wird ausgegeben. Die Montagelöcher H1–H4
sind „nur Platine" und haben kein Symbol. Der DRC mit `--schematic-parity` prüft, dass Platine
und Schaltplan zusammenpassen.

Die Vorplatzierung (`tools/place_controller.py`) lief einmalig für REV A und verweigert einen
zweiten Lauf, sobald ein Platinenumriss existiert.

## Prüfen und exportieren

```bash
"D:/hobby/Elektronik/KiCad/10.0/bin/kicad-cli.exe" pcb drc --schematic-parity --exit-code-violations --output hardware/controller/output/drc.rpt hardware/controller/controller.kicad_pcb
```

```bash
"D:/hobby/Elektronik/KiCad/10.0/bin/kicad-cli.exe" pcb export gerbers --output hardware/controller/output/gerber/ hardware/controller/controller.kicad_pcb
```

```bash
"D:/hobby/Elektronik/KiCad/10.0/bin/kicad-cli.exe" pcb export drill --output hardware/controller/output/gerber/ hardware/controller/controller.kicad_pcb
```

```bash
"D:/hobby/Elektronik/KiCad/10.0/bin/kicad-cli.exe" pcb export pos --output hardware/controller/output/controller-pos.csv hardware/controller/controller.kicad_pcb
```

```bash
"D:/hobby/Elektronik/KiCad/10.0/bin/kicad-cli.exe" pcb export step --output hardware/controller/output/controller.step hardware/controller/controller.kicad_pcb
```

```bash
"D:/hobby/Elektronik/KiCad/10.0/bin/kicad-cli.exe" sch export bom --fields "Reference,Value,Footprint,MPN,Source,QUANTITY,DNP" --group-by "Value,Footprint,MPN" --output hardware/controller/output/controller-bom.csv hardware/controller/controller.kicad_sch
```

## Regeln

- **Jede Pinbelegung und jeden Footprint gegen das Datenblatt prüfen**, bevor er verwendet
  wird; geprüfte Teile in `lib/README.md` eintragen. KI-Werkzeuge machen hier typischerweise
  Fehler.
- Netzspannung führende Netze gehören zur Netzklasse `Mains`; ihre Abstände prüft der DRC
  (Regeln in `controller/controller.kicad_dru`).
- Keine Test- oder Messpunkte im 230-V-Bereich.
- GPIO-Zuordnung im Schaltplan und in `firmware/include/board/board_rev_a.h` müssen
  übereinstimmen (Verfassung, Prinzip VI); `check_schematic.py` prüft das.
- Netze mit Netzklasse (L_IN, N_IN, L_FUSED, Versorgungen) brauchen im Schaltplan ein Label mit
  genau diesem Namen, sonst fallen sie aus ihrer Netzklasse und damit aus den Abstandsregeln.
