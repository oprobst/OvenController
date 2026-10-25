"""Creates the KiCad 10 project file for a board with the oven's net classes and DRC defaults (T019).

Usage (normal Python, no KiCad needed):
    python hardware/tools/make_project.py hardware/controller/controller.kicad_pro

Run after make_board.py (saving a new board resets the project file).
Starts from a KiCad 10 demo project so all sections exist in the current format, then sets
net classes, net class patterns and minimum values (research.md R2, R13). Custom clearance
rules for mains live in <board>.kicad_dru.
"""

import json
import sys
from pathlib import Path

TEMPLATE = Path(r"D:\hobby\Elektronik\KiCad\10.0\share\kicad\demos\stickhub\StickHub.kicad_pro")

MAINS_NETS = ["L_IN", "N_IN", "L_FUSED"]
POWER_NETS = ["+5V", "+5V_PSU", "+3V3", "GND", "VBUS", "+5V_SSR", "RELAY_N", "SSR_TOP_N",
              "SSR_BOT_N", "BUZZER_N", "TFT_BL"]


def netclass(base, name, clearance, track, via_d, via_drill, priority):
    nc = dict(base)
    nc.update(name=name, clearance=clearance, track_width=track, via_diameter=via_d,
              via_drill=via_drill, priority=priority)
    return nc


def main(target):
    target = Path(target)
    pro = json.loads(TEMPLATE.read_text(encoding="utf-8"))
    pro["meta"]["filename"] = target.name
    for key in ("boards", "sheets", "text_variables"):
        pro[key] = [] if isinstance(pro[key], list) else {}
    pro["libraries"] = {"pinned_footprint_libs": [], "pinned_symbol_libs": []}
    pro["pcbnew"]["last_paths"] = {k: "" for k in pro["pcbnew"]["last_paths"]}

    ns = pro["net_settings"]
    default = ns["classes"][0]
    ns["classes"] = [
        netclass(default, "Default", 0.2, 0.25, 0.6, 0.3, 2147483647),
        netclass(default, "Power", 0.2, 0.6, 0.8, 0.4, 1),
        netclass(default, "Mains", 3.0, 1.0, 0.8, 0.4, 0),
    ]
    ns["netclass_patterns"] = (
        [{"netclass": "Mains", "pattern": n} for n in MAINS_NETS]
        + [{"netclass": "Power", "pattern": n} for n in POWER_NETS]
    )

    ds = pro["board"]["design_settings"]
    ds["rules"].update(
        min_clearance=0.2, min_track_width=0.2, min_via_diameter=0.6,
        min_through_hole_diameter=0.3, min_via_annular_width=0.13, min_hole_to_hole=0.25,
        min_copper_edge_clearance=0.5, min_hole_clearance=0.25, min_silk_clearance=0.0,
        min_text_height=0.8, min_text_thickness=0.15,
    )
    ds["track_widths"] = [0.0, 0.25, 0.4, 0.6, 1.0]
    ds["via_dimensions"] = [{"diameter": 0.0, "drill": 0.0}, {"diameter": 0.6, "drill": 0.3},
                            {"diameter": 0.8, "drill": 0.4}]
    ds["drc_exclusions"] = []
    target.write_text(json.dumps(pro, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(f"written {target}")


if __name__ == "__main__":
    main(sys.argv[1])
