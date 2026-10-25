"""Controller REV A: builds all schematic blocks, runs the ERC, writes netlist and BOM (T017).

Usage (from the repository root):
    hardware/.venv/Scripts/python hardware/controller/schematic/main.py

Outputs:
    hardware/controller/controller.net         KiCad netlist (versioned, review basis)
    hardware/controller/output/controller-bom.csv

Fails with exit code 1 on ERC errors/warnings or if the GPIO nets differ from
firmware/include/board/board_rev_a.h (constitution principle VI).
"""

import builtins
import csv
import os
import re
import sys
from collections import defaultdict
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
os.chdir(HERE.parent)  # SKiDL writes main.log / main.erc into the working directory

import common  # noqa: E402,F401  (configures SKiDL before the blocks import it)
import load  # noqa: E402
import mcu  # noqa: E402
import power  # noqa: E402
import safety  # noqa: E402
import thermocouple  # noqa: E402
import ui  # noqa: E402
import usb  # noqa: E402
from skidl import ERC, erc_logger, generate_netlist  # noqa: E402

BOARD_DIR = HERE.parent
REPO = BOARD_DIR.parents[1]
NETLIST = BOARD_DIR / "controller.net"
BOM = BOARD_DIR / "output" / "controller-bom.csv"
BOARD_HEADER = REPO / "firmware" / "include" / "board" / "board_rev_a.h"

# Firmware constant -> schematic net driven/read by that GPIO.
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


def check_board_header():
    """Compares every kPin* constant of board_rev_a.h with the GPIO nets of mcu.py."""
    header = BOARD_HEADER.read_text(encoding="utf-8")
    pins = {m.group(1): int(m.group(2))
            for m in re.finditer(r"constexpr int8_t (kPin\w+) = (\d+);", header)}
    errors = []
    for const, gpio in pins.items():
        expected = HEADER_TO_NET.get(const)
        actual = mcu.GPIO_NETS.get(gpio)
        if expected is None:
            errors.append(f"{const} = {gpio}: no mapping in HEADER_TO_NET")
        elif actual != expected:
            errors.append(f"{const} = GPIO {gpio}: header expects net {expected}, schematic has {actual}")
    for const in HEADER_TO_NET.keys() - pins.keys():
        errors.append(f"{const} missing in {BOARD_HEADER.name}")
    return errors


def write_bom(circuit):
    groups = defaultdict(list)
    for p in circuit.parts:
        key = (p.value, p.footprint, p.fields.get("MPN", ""), p.fields.get("Source", ""),
               p.fields.get("DNP", ""))
        groups[key].append(p.ref)
    BOM.parent.mkdir(exist_ok=True)
    with BOM.open("w", newline="", encoding="utf-8") as f:
        w = csv.writer(f, delimiter=";")
        w.writerow(["Qty", "References", "Value", "Footprint", "MPN", "Source", "DNP"])
        for (value, fp, mpn, source, dnp), refs in sorted(groups.items(), key=lambda g: g[1][0]):
            w.writerow([len(refs), " ".join(sorted(refs)), value, fp, mpn, source, dnp])


def main():
    for block in (power, mcu, usb, thermocouple, safety, load, ui):
        block.build()

    circuit = builtins.default_circuit
    ERC()
    erc_problems = (erc_logger.error.count + erc_logger.bare_error.count
                    + erc_logger.warning.count + erc_logger.bare_warning.count)

    header_errors = check_board_header()
    for e in header_errors:
        print("BOARD HEADER MISMATCH:", e)

    generate_netlist(file_=str(NETLIST))
    write_bom(circuit)
    print(f"parts: {len(circuit.parts)}, netlist: {NETLIST.relative_to(REPO)}, bom: {BOM.relative_to(REPO)}")
    return 1 if header_errors or erc_problems else 0


if __name__ == "__main__":
    sys.exit(main())
