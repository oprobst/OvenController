"""Checks a KiCad schematic against the frozen reference netlist and the firmware board header.

Usage (normal Python; kicad-cli is called for the export):
    python hardware/tools/check_schematic.py controller
    python hardware/tools/check_schematic.py encoder-carrier

1. Exports the netlist of hardware/<board>/<board>.kicad_sch with kicad-cli.
2. Compares it with hardware/<board>/<board>.ref.net (the netlist verified in feature 002,
   generated from the former SKiDL source):
   - parts: same references, values, footprints, MPN and DNP
   - connectivity: the same pins are connected together (net names may differ)
   - net names at the ESP32 pins must match (the firmware pin check depends on them)
   - net names that select a net class in <board>.kicad_pro (Mains, Power) must match: an
     unnamed mains net would silently lose the 6 mm / 3 mm clearance rules
3. Controller only: every kPin* constant in firmware/include/board/board_rev_a.h must sit on the
   expected net at the ESP32 (constitution principle VI).

Exit code 1 on any difference; every difference is listed. Renamed nets elsewhere are reported
as notes. A deliberate change of the circuit means: update the reference netlist in the same
commit, with the reason in the commit message.
"""

import fnmatch
import json
import re
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from sexp import children, parse_sexp, value  # noqa: E402

REPO = Path(__file__).resolve().parents[2]
KICAD_CLI = Path(r"D:\hobby\Elektronik\KiCad\10.0\bin\kicad-cli.exe")
BOARD_HEADER = REPO / "firmware" / "include" / "board" / "board_rev_a.h"
MCU_REF = "U3"

# ESP32-S3-WROOM-1/1U module pin per GPIO (Espressif datasheet v1.8, table 3-1).
MODULE_PIN = {
    4: 4, 5: 5, 6: 6, 7: 7, 15: 8, 16: 9, 17: 10, 18: 11, 8: 12, 19: 13, 20: 14, 3: 15,
    46: 16, 9: 17, 10: 18, 11: 19, 12: 20, 13: 21, 14: 22, 21: 23, 47: 24, 48: 25, 45: 26,
    0: 27, 35: 28, 36: 29, 37: 30, 38: 31, 39: 32, 40: 33, 41: 34, 42: 35, 44: 36, 43: 37,
    2: 38, 1: 39,
}

# Firmware constant -> net at the ESP32 pin of that GPIO.
HEADER_TO_NET = {
    "kPinRevisionStrap": "REV_STRAP", "kPinDiagLed": "LED_DIAG", "kPinHeatTop": "HEAT_TOP",
    "kPinHeatBottom": "HEAT_BOT", "kPinArm": "ARM", "kPinDisarm": "DISARM",
    "kPinWatchdogWdi": "WDI", "kPinClearSense": "CLR_SENSE", "kPinStopNo": "STOP_NO",
    "kPinLatchQ": "LATCH_Q", "kPinTftBacklight": "TFT_BL_PWM", "kPinTftRst": "TFT_RST",
    "kPinTftDc": "TFT_DC", "kPinTftCs": "TFT_CS", "kPinSpiMosi": "TFT_MOSI_MCU",
    "kPinSpiSck": "TFT_SCK_MCU", "kPinThermocoupleMiso": "TC_MISO",
    "kPinThermocoupleSck": "TC_SCK", "kPinThermocoupleMosi": "TC_MOSI",
    "kPinThermocoupleCsTop": "TC_CS_TOP", "kPinThermocoupleCsBottom": "TC_CS_BOT",
    "kPinBuzzer": "BUZZER", "kPinEncoderA": "ENC_A", "kPinEncoderB": "ENC_B",
    "kPinEncoderButton": "ENC_SW",
}


def short(name):
    """Net name without the sheet path, e.g. '/Sicherheit/CLR' -> 'CLR'."""
    return name.rsplit("/", 1)[-1] if name else name


def read_netlist(path):
    root = parse_sexp(Path(path).read_text(encoding="utf-8"))
    parts = {}
    for comp in children(children(root, "components")[0], "comp"):
        fields = {}
        for f in children(children(comp, "fields")[0], "field") if children(comp, "fields") else []:
            if len(f) > 2 and isinstance(f[2], str):
                fields[value(f, "name")] = f[2]
        parts[value(comp, "ref")] = (value(comp, "value"), value(comp, "footprint"),
                                     fields.get("MPN", ""), fields.get("DNP", ""))
    nets = {}
    for net in children(children(root, "nets")[0], "net"):
        nodes = frozenset((value(n, "ref"), value(n, "pin")) for n in children(net, "node"))
        if len(nodes) > 1:  # single pins (no-connects) carry no connectivity
            nets[nodes] = short(value(net, "name"))
    return parts, nets


def export_netlist(sch, out):
    subprocess.run([str(KICAD_CLI), "sch", "export", "netlist", "--output", str(out), str(sch)],
                   check=True, capture_output=True)


def netclass_patterns(board_dir, board):
    pro = board_dir / f"{board}.kicad_pro"
    if not pro.exists():
        return []
    patterns = json.loads(pro.read_text(encoding="utf-8"))["net_settings"].get("netclass_patterns") or []
    return [(p["pattern"], p["netclass"]) for p in patterns]


def compare(ref, sch, patterns):
    errors, notes = [], []
    ref_parts, ref_nets = ref
    sch_parts, sch_nets = sch
    for r in sorted(ref_parts.keys() - sch_parts.keys()):
        errors.append(f"part missing in schematic: {r} {ref_parts[r][0]}")
    for r in sorted(sch_parts.keys() - ref_parts.keys()):
        errors.append(f"extra part in schematic: {r} {sch_parts[r][0]}")
    labels = ("value", "footprint", "MPN", "DNP")
    for r in sorted(ref_parts.keys() & sch_parts.keys()):
        for label, a, b in zip(labels, ref_parts[r], sch_parts[r]):
            if a != b:
                errors.append(f"{r}: {label} '{b}' in schematic, '{a}' in reference")

    pin_net = {pin: nodes for nodes in sch_nets for pin in nodes}
    for nodes, name in sorted(ref_nets.items(), key=lambda kv: kv[1]):
        if nodes in sch_nets:
            if sch_nets[nodes] != name:
                msg = f"net {name} is called {sch_nets[nodes]} in the schematic"
                classes = [cls for pat, cls in patterns if fnmatch.fnmatchcase(name, pat)]
                if classes:
                    errors.append(f"{msg}: loses net class {classes[0]}")
                elif any(ref == MCU_REF for ref, _ in nodes):
                    errors.append(msg)
                else:
                    notes.append(msg)
            continue
        groups = {pin_net.get(p) for p in nodes}
        detail = ", ".join(f"{r}.{p}" for r, p in sorted(nodes))
        if len(groups) > 1 or None in groups:
            errors.append(f"net {name} is split or incomplete in the schematic: {detail}")
        else:
            extra = sorted(next(iter(groups)) - nodes)
            errors.append(f"net {name} has extra pins in the schematic: "
                          + ", ".join(f"{r}.{p}" for r, p in extra))
    return errors, notes


def check_board_header(sch_nets):
    header = BOARD_HEADER.read_text(encoding="utf-8")
    pins = {m.group(1): int(m.group(2))
            for m in re.finditer(r"constexpr int8_t (kPin\w+) = (\d+);", header)}
    at_pin = {pin: name for nodes, name in sch_nets.items() for pin in nodes}
    errors = []
    for const, gpio in pins.items():
        expected = HEADER_TO_NET.get(const)
        actual = at_pin.get((MCU_REF, str(MODULE_PIN[gpio])))
        if expected is None:
            errors.append(f"{const} = {gpio}: no expected net in check_schematic.py")
        elif actual != expected:
            errors.append(f"{const} = GPIO {gpio}: header expects net {expected}, schematic has {actual}")
    for const in HEADER_TO_NET.keys() - pins.keys():
        errors.append(f"{const} missing in {BOARD_HEADER.name}")
    return errors


def main(board):
    board_dir = REPO / "hardware" / board
    with tempfile.TemporaryDirectory() as tmp:
        out = Path(tmp) / f"{board}.net"
        export_netlist(board_dir / f"{board}.kicad_sch", out)
        sch = read_netlist(out)
    ref = read_netlist(board_dir / f"{board}.ref.net")
    errors, notes = compare(ref, sch, netclass_patterns(board_dir, board))
    if board == "controller":
        errors += check_board_header(sch[1])
    for n in notes:
        print("NOTE:", n)
    for e in errors:
        print("ERROR:", e)
    print(f"{board}: {len(sch[0])} parts, {len(sch[1])} nets, {len(errors)} errors, {len(notes)} notes")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1]))
