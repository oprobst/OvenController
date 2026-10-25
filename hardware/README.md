# Hardware

KiCad-Projekte des Ofen-Controllers. Entscheidungen und Begründungen:
`specs/002-controller-pcb/plan.md` und `research.md`, Werkzeugwahl: `docs/werkzeuge-pcb.md`.

| Verzeichnis | Inhalt |
|---|---|
| `lib/` | projekteigene Symbole (`oven.kicad_sym`) und Footprints (`oven.pretty/`) |
| `controller/` | Hauptplatine REV A: Schaltplan als SKiDL-Code in `schematic/`, KiCad-Projekt, Netzliste, Exporte in `output/` |
| `encoder-carrier/` | Encoder-Träger REV A, gleiche Struktur |

## Werkzeugkette

- **Schaltplan**: Python-Code mit [SKiDL](https://github.com/devbisme/skidl), erzeugt eine
  KiCad-Netzliste (`*.net`). Die Netzliste wird mitversioniert, damit Reviews ohne Python
  möglich sind.
- **Layout, ERC/DRC, Exporte**: KiCad 10.0.3, installiert unter
  `D:\hobby\Elektronik\KiCad\10.0`. Kommandozeile: `D:\hobby\Elektronik\KiCad\10.0\bin\kicad-cli.exe`.
- Das Layout entsteht in KiCad aus der Netzliste („Platine aus Netzliste aktualisieren").
  Kein Werkzeug außer KiCad schreibt in `.kicad_pcb`.

## Einrichtung (einmalig)

```bash
python -m venv hardware/.venv
```

```bash
hardware/.venv/Scripts/python -m pip install -r hardware/requirements.txt
```

## Netzliste erzeugen

```bash
hardware/.venv/Scripts/python hardware/controller/schematic/main.py
```

Das Skript führt den ERC von SKiDL aus, gleicht die GPIO-Netze mit
`firmware/include/board/board_rev_a.h` ab und schreibt `hardware/controller/controller.net` sowie
`hardware/controller/output/controller-bom.csv`. Exit-Code 1 bei ERC-Meldungen oder
Abweichungen zum Board-Header. Harmlose Ausgaben: Warnungen zu `KICAD*_SYMBOL_DIR` älterer
Versionen, zur globalen `fp-lib-table` und ein „Missing tag" ohne Bauteilnamen (SKiDL-intern).

Netzliste gegen KiCad 10 prüfen (alle Footprints ladbar, jeder Pin hat ein Pad):

```bash
"D:/hobby/Elektronik/KiCad/10.0/bin/python.exe" hardware/tools/netlist_check.py hardware/controller/controller.net
```

## Prüfen und exportieren

```bash
"D:/hobby/Elektronik/KiCad/10.0/bin/kicad-cli.exe" pcb drc --exit-code-violations --output hardware/controller/output/drc.rpt hardware/controller/controller.kicad_pcb
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

Die Stückliste erzeugt das SKiDL-Skript aus den Bauteilfeldern (Hersteller, Bestellnummer,
Quelle) als `output/controller-bom.csv`.

## Regeln

- **Jede Pinbelegung und jeden Footprint gegen das Datenblatt prüfen**, bevor er verwendet
  wird; geprüfte Teile in `lib/README.md` eintragen. KI-Werkzeuge machen hier typischerweise
  Fehler.
- Netzspannung führende Netze gehören zur Netzklasse `Mains`; ihre Abstände prüft der DRC
  (Regeln in `controller/controller.kicad_dru`).
- Keine Test- oder Messpunkte im 230-V-Bereich.
- GPIO-Zuordnung in der Netzliste und in `firmware/include/board/board_rev_a.h` müssen
  übereinstimmen (Verfassung, Prinzip VI).
