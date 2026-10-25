"""Updates a board from a SKiDL netlist, like KiCad's "Update PCB from Netlist" (T020).

Run with KiCad's bundled Python:
    "D:/hobby/Elektronik/KiCad/10.0/bin/python.exe" hardware/tools/import_netlist.py hardware/controller/controller.net hardware/controller/controller.kicad_pcb

- Footprints are matched by their symbol path (the netlist tstamp, stable through SKiDL tags).
- Existing footprints keep position, side and rotation; reference, value, fields and nets are
  updated. A changed footprint ID is reported, not swapped (do that in KiCad).
- New footprints are placed in a row below the board; footprints whose symbol is gone are
  removed. Parts without a symbol path (mounting holes, logos) are never touched.
- The project file is restored afterwards (saving resets it, see make_board.py).
"""

import shutil
import sys
from pathlib import Path

import pcbnew

sys.path.insert(0, str(Path(__file__).resolve().parent))
from netlist_check import KICAD_FP, PROJECT_LIBS, children, parse_sexp, value  # noqa: E402

NEW_PARTS_Y_MM = 130.0
NEW_PARTS_STEP_MM = 6.0


def fields_of(comp):
    result = {}
    for f in children(children(comp, "fields")[0], "field") if children(comp, "fields") else []:
        name = value(f, "name")
        if name and len(f) > 2 and isinstance(f[2], str):
            result[name] = f[2]
    return result


def load_footprint(fp_id):
    lib, name = fp_id.split(":", 1)
    lib_dir = PROJECT_LIBS.get(lib, KICAD_FP / f"{lib}.pretty")
    fp = pcbnew.FootprintLoad(str(lib_dir), name)
    if fp is not None:
        fp.SetFPID(pcbnew.LIB_ID(lib, name))
    return fp


def main(netlist, board_path):
    board_path = Path(board_path)
    pro = board_path.with_suffix(".kicad_pro")
    pro_backup = pro.read_bytes()
    root = parse_sexp(Path(netlist).read_text(encoding="utf-8"))
    board = pcbnew.LoadBoard(str(board_path))

    existing = {fp.GetPath().AsString(): fp for fp in board.GetFootprints()
                if fp.GetPath().AsString()}
    wanted = {}
    added = updated = 0
    x = 0.0
    for comp in children(children(root, "components")[0], "comp"):
        ref, fp_id, val = value(comp, "ref"), value(comp, "footprint"), value(comp, "value")
        path = "/" + value(comp, "tstamps")
        wanted[path] = ref
        fp = existing.get(path)
        if fp is None:
            fp = load_footprint(fp_id)
            if fp is None:
                sys.exit(f"footprint not found: {ref} {fp_id}")
            fp.SetPath(pcbnew.KIID_PATH(path))
            fp.SetPosition(pcbnew.VECTOR2I(pcbnew.FromMM(x), pcbnew.FromMM(NEW_PARTS_Y_MM)))
            x += NEW_PARTS_STEP_MM
            board.Add(fp)
            added += 1
        else:
            fpid = fp.GetFPID()
            current = f"{fpid.GetLibNickname()}:{fpid.GetLibItemName()}"
            if current != fp_id:
                print(f"NOTE {ref}: footprint changed {current} -> {fp_id}, swap in KiCad")
            updated += 1
        fp.SetReference(ref)
        fp.SetValue(val or "")
        for name, text in fields_of(comp).items():
            if name in ("MPN", "Source", "DNP"):
                fp.SetField(name, text)
                fp.GetField(name).SetVisible(False)  # BOM data, not for the silkscreen
        fp.SetDNP(fields_of(comp).get("DNP") == "yes")

    removed = 0
    for path, fp in existing.items():
        if path not in wanted:
            board.Remove(fp)
            removed += 1

    by_ref = {fp.GetReference(): fp for fp in board.GetFootprints()}
    for fp in board.GetFootprints():
        if fp.GetPath().AsString():
            for pad in fp.Pads():
                pad.SetNetCode(0)
    for net_node in children(children(root, "nets")[0], "net"):
        name = value(net_node, "name")
        net = board.FindNet(name)
        if net is None:
            net = pcbnew.NETINFO_ITEM(board, name)
            board.Add(net)
        for node in children(net_node, "node"):
            fp = by_ref[value(node, "ref")]
            for pad in fp.Pads():
                if pad.GetNumber() == value(node, "pin"):
                    pad.SetNet(net)

    pcbnew.SaveBoard(str(board_path), board)
    pro.write_bytes(pro_backup)
    print(f"added {added}, updated {updated}, removed {removed}")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
