"""Generates the fixed screw terminal footprints for J1 (mains) and J2 (SSR/relay).

Body, silkscreen, courtyard and 3D model come from the KiCad library footprints of the Phoenix
MKDS terminals; the pads are taken over from the former pluggable headers (MSTBA 2,5 / MC 1,5):
oval pads keep the narrow width across the pitch, so J1 keeps 3 mm between L and N (rule
mains_l_to_n) and J2 keeps the clearance to the tracks routed for the old footprint.

Run with KiCad's bundled Python:
    "D:/hobby/Elektronik/KiCad/10.0/bin/python.exe" hardware/tools/make_terminal_footprints.py
"""
from pathlib import Path

import pcbnew

KICAD_FP = Path(r"D:\hobby\Elektronik\KiCad\10.0\share\kicad\footprints")
OUT = Path(__file__).resolve().parents[1] / "lib" / "oven.pretty"

# name: (body footprint, footprint with the pads, description)
TERMINALS = {
    "Phoenix_MKDS-1,5-2-5.08_Horizontal_OvalPads": (
        ("TerminalBlock_Phoenix", "TerminalBlock_Phoenix_MKDS-1,5-2-5.08_1x02_P5.08mm_Horizontal"),
        ("Connector_Phoenix_MSTB", "PhoenixContact_MSTBA_2,5_2-G-5,08_1x02_P5.08mm_Horizontal"),
        "Phoenix MKDS 1,5/2-5,08 screw terminal, pitch 5.08 mm, pads of MSTBA 2,5/2-G-5,08 "
        "(oval 2.08 x 3.6 mm, drill 1.4 mm) for 3 mm L-N clearance"),
    "Phoenix_MKDS-1-6-3.81_Horizontal_OvalPads": (
        ("TerminalBlock_Phoenix", "TerminalBlock_Phoenix_MKDS-1-6-3.81_1x06_P3.81mm_Horizontal"),
        ("Connector_Phoenix_MC", "PhoenixContact_MC_1,5_6-G-3.81_1x06_P3.81mm_Horizontal"),
        "Phoenix MKDS 1/6-3,81 screw terminal, pitch 3.81 mm, pads of MC 1,5/6-G-3,81 "
        "(oval 1.8 x 3.6 mm, drill 1.2 mm)"),
}


def load(lib, name):
    return pcbnew.FootprintLoad(str(KICAD_FP / f"{lib}.pretty"), name)


def main():
    for name, (body_id, pads_id, descr) in TERMINALS.items():
        fp = load(*body_id)
        pads = {p.GetNumber(): p for p in load(*pads_id).Pads()}
        for p in fp.Pads():
            src = pads[p.GetNumber()]
            assert p.GetPosition() == src.GetPosition(), (name, p.GetNumber())
            p.SetShape(pcbnew.F_Cu, src.GetShape(pcbnew.F_Cu))
            p.SetSize(pcbnew.F_Cu, src.GetSize(pcbnew.F_Cu))
            p.SetRoundRectRadiusRatio(pcbnew.F_Cu, src.GetRoundRectRadiusRatio(pcbnew.F_Cu))
            p.SetDrillSize(src.GetDrillSize())
        fp.SetFPID(pcbnew.LIB_ID("oven", name))
        fp.SetLibDescription(descr)
        fp.SetKeywords("Phoenix MKDS screw terminal fixed")
        pcbnew.FootprintSave(str(OUT), fp)
        print("written:", OUT / f"{name}.kicad_mod")


if __name__ == "__main__":
    main()
