"""Initial placement of controller REV A (T020): outline, mounting holes, slot, zones.

Run once with KiCad's bundled Python after import_netlist.py:
    "D:/hobby/Elektronik/KiCad/10.0/bin/python.exe" hardware/tools/place_controller.py hardware/controller/controller.net hardware/controller/controller.kicad_pcb

This is a starting point for the layout in KiCad, not the final placement. The script refuses
to run if the board already has an outline, so manual work is never overwritten.

Board 100 x 100 mm, coordinates in mm, x to the right, y down. Edges: left = rear of the oven
(mains inlet), right = front panel (display, control panel), top = up (USB, antenna), bottom =
thermocouple sockets. Zones (research.md R13): mains column x < 17 plus the IRM-10-5 primary
side; everything else is low voltage at x >= 23 (>= 6 mm from mains copper). A milled slot
under the IRM-10-5 separates its primary and secondary pins.
"""

import sys
from pathlib import Path

import pcbnew

sys.path.insert(0, str(Path(__file__).resolve().parent))
from netlist_check import children, parse_sexp, value  # noqa: E402

BOARD_W, BOARD_H = 100.0, 100.0
MOUNTING_HOLES = [(4, 4), (96, 4), (4, 96), (96, 96)]
SLOT = (25.0, 60.0, 27.0, 94.0)  # x0, y0, x1, y1 of the milled slot under the IRM-10-5

# tag -> (centre x, centre y, rotation in degrees) for parts with a fixed place
ANCHORS = {
    "pwr.J1": (9, 16, 90),
    "pwr.F1": (10, 40, 270),  # input (L_IN) towards J1, output (L_FUSED) towards the IRM
    "pwr.RV1": (10, 62, 0),
    "pwr.PS1": (26, 78, 0),
    "load.J2": (36, 6, 0),
    "mcu.U1": (62, 15, 0),
    "usb.J7": (82, 6, 180),
    "mcu.SW1": (93, 16, 0),
    "mcu.SW2": (93, 26, 0),
    "ui.J3": (95, 44, 0),
    "ui.J4": (95, 67, 0),
    "ui.BZ1": (81, 58, 0),
    "tc.top.J1": (61, 88, 0),
    "tc.bot.J1": (82, 88, 0),
    "tc.top.U1": (61, 74, 0),
    "tc.bot.U1": (82, 74, 0),
    "saf.U3": (49, 60, 0),   # TC6501 at the hot spot between IRM output and LDO
    "pwr.U1": (42, 59, 0),   # LDO
}

# tag prefix -> region (x0, y0, x1, y1) for the remaining parts of that block
REGIONS = [
    ("pwr.", (28, 45, 56, 64)),
    ("load.", (24, 13, 52, 26)),
    ("saf.", (24, 28, 52, 44)),
    ("mcu.", (52, 27, 86, 40)),
    ("usb.", (70, 12, 88, 26)),
    ("tc.", (52, 64, 72, 78)),
    ("ui.", (56, 41, 89, 66)),
]


def tags_by_ref(netlist):
    root = parse_sexp(Path(netlist).read_text(encoding="utf-8"))
    result = {}
    for comp in children(children(root, "components")[0], "comp"):
        for f in children(children(comp, "fields")[0], "field"):
            if value(f, "name") == "SKiDL Tag":
                result[value(comp, "ref")] = f[2]
    return result


def mm(v):
    return pcbnew.FromMM(v)


def bbox(fp):
    crt = fp.GetCourtyard(pcbnew.F_CrtYd)
    return crt.BBox() if crt.OutlineCount() else fp.GetBoundingBox(False)


def place_centre(fp, cx, cy, rot):
    fp.SetOrientationDegrees(rot)
    bb = bbox(fp)
    centre = bb.GetCenter()
    pos = fp.GetPosition()
    fp.SetPosition(pcbnew.VECTOR2I(pos.x + mm(cx) - centre.x, pos.y + mm(cy) - centre.y))


def add_rect(board, x0, y0, x1, y1):
    s = pcbnew.PCB_SHAPE(board)
    s.SetShape(pcbnew.SHAPE_T_RECT)
    s.SetLayer(pcbnew.Edge_Cuts)
    s.SetWidth(mm(0.1))
    s.SetStart(pcbnew.VECTOR2I(mm(x0), mm(y0)))
    s.SetEnd(pcbnew.VECTOR2I(mm(x1), mm(y1)))
    board.Add(s)


def add_mounting_hole(board, x, y, n):
    lib = r"D:\hobby\Elektronik\KiCad\10.0\share\kicad\footprints\MountingHole.pretty"
    fp = pcbnew.FootprintLoad(lib, "MountingHole_3.2mm_M3")
    fp.SetFPID(pcbnew.LIB_ID("MountingHole", "MountingHole_3.2mm_M3"))
    fp.SetReference(f"H{n}")
    fp.SetPosition(pcbnew.VECTOR2I(mm(x), mm(y)))
    board.Add(fp)


def overlaps(rect, occupied, gap):
    x0, y0, x1, y1 = rect
    return any(x0 < ox1 + gap and x1 > ox0 - gap and y0 < oy1 + gap and y1 > oy0 - gap
               for ox0, oy0, ox1, oy1 in occupied)


def rect_mm(fp):
    bb = bbox(fp)
    return (pcbnew.ToMM(bb.GetLeft()), pcbnew.ToMM(bb.GetTop()),
            pcbnew.ToMM(bb.GetRight()), pcbnew.ToMM(bb.GetBottom()))


def shelf_pack(fps, region, occupied):
    """Places the parts row by row inside region, skipping occupied areas (gap 0.8 mm)."""
    x0, y0, x1, y1 = region
    gap, step = 0.8, 0.5
    for fp in sorted(fps, key=lambda f: -pcbnew.ToMM(bbox(f).GetHeight())):
        w, h = pcbnew.ToMM(bbox(fp).GetWidth()), pcbnew.ToMM(bbox(fp).GetHeight())
        spot = None
        y = y0
        while spot is None and y + h <= y1:
            x = x0
            while x + w <= x1:
                if not overlaps((x, y, x + w, y + h), occupied, gap):
                    spot = (x, y)
                    break
                x += step
            y += step
        if spot is None:
            print(f"region full: {fp.GetReference()} left below the board")
            continue
        place_centre(fp, spot[0] + w / 2, spot[1] + h / 2, 0)
        occupied.append(rect_mm(fp))


def main(netlist, board_path):
    board_path = Path(board_path)
    pro = board_path.with_suffix(".kicad_pro")
    pro_backup = pro.read_bytes()
    board = pcbnew.LoadBoard(str(board_path))
    if any(d.GetLayer() == pcbnew.Edge_Cuts for d in board.GetDrawings()):
        sys.exit("board already has an outline, initial placement not repeated")

    add_rect(board, 0, 0, BOARD_W, BOARD_H)
    add_rect(board, *SLOT)
    for n, (x, y) in enumerate(MOUNTING_HOLES, start=1):
        add_mounting_hole(board, x, y, n)

    tags = tags_by_ref(netlist)
    rest = {prefix: [] for prefix, _ in REGIONS}
    occupied = [(x - 3.5, y - 3.5, x + 3.5, y + 3.5) for x, y in MOUNTING_HOLES]  # screw heads
    occupied.append((SLOT[0] - 1, SLOT[1], SLOT[2] + 1, SLOT[3]))
    for fp in board.GetFootprints():
        tag = tags.get(fp.GetReference())
        if tag is None:
            continue
        if tag in ANCHORS:
            place_centre(fp, *ANCHORS[tag])
            occupied.append(rect_mm(fp))
            continue
        for prefix, _ in REGIONS:
            if tag.startswith(prefix):
                rest[prefix].append(fp)
                break
    for prefix, region in REGIONS:
        shelf_pack(rest[prefix], region, occupied)

    pcbnew.SaveBoard(str(board_path), board)
    pro.write_bytes(pro_backup)
    print("initial placement written")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
