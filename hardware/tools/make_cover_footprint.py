"""Generates the board-only footprint of the mains cover (T083, research R19).

The footprint `oven:Netzabdeckung_REVA` (reference COV1) is placed at the board origin (0, 0), so
all coordinates below are board coordinates in mm. It holds:
- the outline of the top and bottom shell on F.Fab / B.Fab (with overhang beyond the board edge),
- rule areas "no footprints" on both sides under the inner wall (wall + tolerance) and under the
  screw flanges,
- the NPTH holes H5-H8 for the polyamide M3 screws,
- sleeve outlines around the mounting holes inside the cover (H1, H3),
- the 3D models of both shells (exported from Fusion 360, T085).

Edit the parameters when the placement changes (T084) and run again with KiCad's Python:
    "D:/hobby/Elektronik/KiCad/10.0/bin/python.exe" hardware/tools/make_cover_footprint.py
Then update the footprint on the board (KiCad: Tools > Update Footprints from Library) and run
the DRC: rule area violations show parts under the wall, `cover_holes_to_mains` the hole distance.
"""

import uuid
from pathlib import Path

OUT = Path(__file__).resolve().parents[1] / "lib" / "oven.pretty" / "Netzabdeckung_REVA.kicad_mod"

# --- parameters (board coordinates, mm) -------------------------------------------------------
OVERHANG = 3.0        # shells reach beyond the board edge (step joint around the edge)
BOARD = (0.0, 0.0, 100.0, 100.0)
WALL = 2.0            # inner wall thickness
TOL = 0.5             # placement tolerance on both sides of the inner wall
# Inner side of the inner wall: column with J1, F1, RV1 (x < COLUMN_X) and band with the
# whole IRM-10-5 (y > BAND_Y, x < BAND_X). Clear of the courtyards of F1 (x 16.3) and U1 (x 49.15,
# y 65.0) by TOL.
COLUMN_X = 16.8
BAND_Y = 64.5
BAND_X = 49.65
SCREW_DRILL = 3.2
FLANGE_R = 4.0        # flange of the shells around a screw (M3 nut, 5.5 mm across flats)
SCREWS = {            # >= 6 mm from mains copper (DRC rule cover_holes_to_mains)
    "H5": (20.8, 7.0),
    "H6": (20.8, 56.0),
    "H7": (55.0, 61.0),
    # Inside the cover below the IRM-10-5 (boss on the inner side of the right wall): outside,
    # the flange would block the thermocouple sockets J5/J6, which need the whole bottom edge
    # between the wall and H4 (T084).
    "H8": (46.5, 96.0),
}
SLEEVES = {"H1": (4.0, 4.0), "H3": (4.0, 96.0)}  # mounting holes inside the cover
SLEEVE_R = 3.5
CABLE = (0.0, 16.0)   # cable entry of J1 at the left board edge
MODELS = ("${KIPRJMOD}/cover/netzabdeckung_oben.step", "${KIPRJMOD}/cover/netzabdeckung_unten.step")


def fmt(v):
    v = round(v, 4)
    return f"{v:.4f}".rstrip("0").rstrip(".") if v else "0"


def uid():
    return f'(uuid "{uuid.uuid4()}")'


def poly(points, layer, width=0.1):
    pts = " ".join(f"(xy {fmt(x)} {fmt(y)})" for x, y in points)
    return (f"\t(fp_poly (pts {pts}) (stroke (width {width}) (type solid)) (fill no) "
            f'(layer "{layer}") {uid()})')


def circle(center, r, layer, width=0.1):
    x, y = center
    return (f"\t(fp_circle (center {fmt(x)} {fmt(y)}) (end {fmt(x + r)} {fmt(y)}) "
            f'(stroke (width {width}) (type solid)) (fill no) (layer "{layer}") {uid()})')


def text(s, at, layer, size=1.0):
    return (f'\t(fp_text user "{s}" (at {fmt(at[0])} {fmt(at[1])} 0) (layer "{layer}") '
            f"(effects (font (size {size} {size}) (thickness {size * 0.15:.2f}))) {uid()})")


def rule_area(name, points):
    pts = " ".join(f"(xy {fmt(x)} {fmt(y)})" for x, y in points)
    return (f'\t(zone (layers "F.Cu" "B.Cu") {uid()} (name "{name}") (hatch edge 0.5) '
            f"(connect_pads (clearance 0)) (min_thickness 0.25) "
            f"(keepout (tracks allowed) (vias allowed) (pads allowed) (copperpour allowed) "
            f"(footprints not_allowed)) (placement (enabled no) (sheetname \"\")) "
            f"(fill (thermal_gap 0.5) (thermal_bridge_width 0.5) (island_removal_mode 0)) "
            f"(polygon (pts {pts})))")


def circle_points(center, r, n=24):
    import math
    return [(center[0] + r * math.cos(2 * math.pi * k / n), center[1] + r * math.sin(2 * math.pi * k / n))
            for k in range(n)]


def main():
    x0, y0, x1, y1 = BOARD
    outer_left, outer_top, outer_bottom = x0 - OVERHANG, y0 - OVERHANG, y1 + OVERHANG
    wall_x = COLUMN_X + WALL       # outer side of the column wall
    wall_y = BAND_Y - WALL         # outer side of the band wall
    wall_bx = BAND_X + WALL        # outer side of the right wall
    outline = [(outer_left, outer_top), (wall_x, outer_top), (wall_x, wall_y), (wall_bx, wall_y),
               (wall_bx, outer_bottom), (outer_left, outer_bottom)]
    # keepout under the inner wall (wall + tolerance on both sides), from the top edge to the bottom edge
    a, b = COLUMN_X - TOL, wall_x + TOL
    c, d = wall_y - TOL, BAND_Y + TOL
    e, f = BAND_X - TOL, wall_bx + TOL
    wall_area = [(a, y0), (b, y0), (b, c), (f, c), (f, y1), (e, y1), (e, d), (a, d)]

    items = []
    for layer in ("F.Fab", "B.Fab"):
        items.append(poly(outline, layer))
    items.append(rule_area("Netzabdeckung Wand", wall_area))
    for name, pos in SCREWS.items():
        items.append(rule_area(f"Netzabdeckung Flansch {name}", circle_points(pos, FLANGE_R)))
        for layer in ("F.Fab", "B.Fab"):
            items.append(circle(pos, FLANGE_R, layer))
    for name, pos in SLEEVES.items():
        for layer in ("F.Fab", "B.Fab"):
            items.append(circle(pos, SLEEVE_R, layer))
        items.append(text(f"Hülse {name}", (pos[0] + 5, pos[1]), "F.Fab", 0.8))
    items.append(text("Netzabdeckung (FR-039)", (COLUMN_X / 2, 30.0), "F.Fab", 1.0))
    items.append(text("Kabel J1", (CABLE[0] - 1.5, CABLE[1]), "F.Fab", 0.8))
    pads = [f'\t(pad "{name}" np_thru_hole circle (at {fmt(x)} {fmt(y)}) (size {fmt(SCREW_DRILL)} {fmt(SCREW_DRILL)}) '
            f'(drill {fmt(SCREW_DRILL)}) (layers "*.Cu" "*.Mask") {uid()})' for name, (x, y) in SCREWS.items()]
    models = [f'\t(model "{m}" (offset (xyz 0 0 0)) (scale (xyz 1 1 1)) (rotate (xyz 0 0 0)))' for m in MODELS]

    descr = ("Mains cover (research R19, FR-039): outline of top/bottom shell on F.Fab/B.Fab, "
             "no-footprint rule areas under the inner wall and the screw flanges, NPTH screw holes "
             "H5-H8, sleeves at H1/H3. Place at the board origin. Generated by "
             "hardware/tools/make_cover_footprint.py")
    head = [
        '(footprint "Netzabdeckung_REVA"',
        "\t(version 20260206)",
        '\t(generator "make_cover_footprint")',
        '\t(layer "F.Cu")',
        f'\t(descr "{descr}")',
        '\t(tags "mains cover 230V shield")',
        '\t(property "Reference" "COV1" (at 2 -1.5 0) (layer "F.Fab") (hide yes) '
        "(effects (font (size 1 1) (thickness 0.15))))",
        '\t(property "Value" "Netzabdeckung_REVA" (at 2 102 0) (layer "F.Fab") (hide yes) '
        "(effects (font (size 1 1) (thickness 0.15))))",
        "\t(attr board_only exclude_from_pos_files exclude_from_bom)",
    ]
    OUT.write_text("\n".join(head + items + pads + models) + "\n)\n", encoding="utf-8")
    print(f"written {OUT}")


if __name__ == "__main__":
    main()
