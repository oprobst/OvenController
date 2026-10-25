"""Checks a SKiDL netlist against KiCad 10's own footprint loader.

Run with KiCad's bundled Python:
    "D:/hobby/Elektronik/KiCad/10.0/bin/python.exe" hardware/tools/netlist_check.py hardware/controller/controller.net

Loads every footprint the way "Update PCB from netlist" does (KiCad 10 libraries plus the project
library hardware/lib/oven.pretty), assigns the nets to the pads and reports footprints that
cannot be found and pins without a matching pad. Exit code 1 on any problem.
"""

import re
import sys
from pathlib import Path

import pcbnew

KICAD_FP = Path(r"D:\hobby\Elektronik\KiCad\10.0\share\kicad\footprints")
PROJECT_LIBS = {"oven": Path(__file__).resolve().parents[1] / "lib" / "oven.pretty"}


def parse_sexp(text):
    tokens = re.findall(r'\(|\)|"(?:[^"\\]|\\.)*"|[^\s()]+', text)
    stack = [[]]
    for tok in tokens:
        if tok == "(":
            stack.append([])
        elif tok == ")":
            done = stack.pop()
            stack[-1].append(done)
        else:
            stack[-1].append(tok[1:-1] if tok.startswith('"') else tok)
    return stack[0][0]


def children(node, key):
    return [c for c in node[1:] if isinstance(c, list) and c and c[0] == key]


def value(node, key):
    found = children(node, key)
    return found[0][1] if found and len(found[0]) > 1 else None


def main(netlist):
    root = parse_sexp(Path(netlist).read_text(encoding="utf-8"))
    problems = 0
    footprints = {}
    for comp in children(children(root, "components")[0], "comp"):
        ref, fp_id = value(comp, "ref"), value(comp, "footprint")
        lib, name = fp_id.split(":", 1)
        lib_dir = PROJECT_LIBS.get(lib, KICAD_FP / f"{lib}.pretty")
        fp = pcbnew.FootprintLoad(str(lib_dir), name)
        if fp is None:
            print(f"FOOTPRINT NOT FOUND: {ref} {fp_id}")
            problems += 1
            continue
        footprints[ref] = fp

    nets = children(children(root, "nets")[0], "net")
    for net_node in nets:
        for node in children(net_node, "node"):
            ref, pin = value(node, "ref"), value(node, "pin")
            fp = footprints.get(ref)
            if fp is not None and not [p for p in fp.Pads() if p.GetNumber() == pin]:
                print(f"NO PAD: {ref} pin {pin} (net {value(net_node, 'name')})")
                problems += 1

    print(f"components: {len(footprints)}, nets: {len(nets)}, problems: {problems}")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1]))
