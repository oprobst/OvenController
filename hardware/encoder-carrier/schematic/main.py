"""Encoder carrier REV A (T018): Alps EC11E15244G1 on a small board behind the front panel.

Usage (from the repository root):
    hardware/.venv/Scripts/python hardware/encoder-carrier/schematic/main.py

The board is held only by the encoder's nut in the front panel. J1 (IDC 2x4) carries the same
pinout as J4 on the controller (contracts/connectors.md), so one flat cable connects both 1:1.
J2 (JST-XH 1x04) takes the wires of the stop button: normally-closed contact (pins 1/2) and
normally-open contact (pins 3/4). Filters and ESD protection sit on the controller (research R9).

Writes hardware/encoder-carrier/encoder-carrier.net; exit code 1 on ERC problems or if the
pinout differs from the controller's J4.
"""

import builtins
import os
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
BOARD_DIR = HERE.parent
REPO = BOARD_DIR.parents[1]
sys.path.insert(0, str(REPO / "hardware" / "controller" / "schematic"))
os.chdir(BOARD_DIR)  # SKiDL writes main.log / main.erc into the working directory

from common import gnd, net, part  # noqa: E402
from skidl import ERC, erc_logger, generate_netlist  # noqa: E402

NETLIST = BOARD_DIR / "encoder-carrier.net"

# Pin of the IDC connector -> signal; must equal J4 in hardware/controller/schematic/ui.py.
IDC_PINOUT = {
    1: "ENC_A_RAW", 2: "GND", 3: "ENC_B_RAW", 4: "ENC_SW_RAW",
    5: "STOP_NC_OUT", 6: "STOP_NC_RET", 7: "STOP_NO_RAW", 8: "GND",
}


def build():
    g = gnd()

    enc = part("Device", "RotaryEncoder_Switch", "enc.SW1",
               "Rotary_Encoder:RotaryEncoder_Alps_EC11E-Switch_Vertical_H20mm",
               value="EC11E15244G1", mpn="Alps EC11E15244G1", source="Conrad 3523314",
               ref="SW1")
    a, b, sw = net("ENC_A_RAW"), net("ENC_B_RAW"), net("ENC_SW_RAW")
    a += enc["A"]
    b += enc["B"]
    g += enc["C"], enc["S2"]   # common of the encoder and of the push switch
    sw += enc["S1"]

    j1 = part("Connector_Generic", "Conn_02x04_Odd_Even", "enc.J1",
              "Connector_IDC:IDC-Header_2x04_P2.54mm_Vertical", value="PANEL",
              mpn="Wannenstecker 2x4 RM2.54", ref="J1")
    for pin, name in IDC_PINOUT.items():
        n = g if name == "GND" else net(name)
        n += j1[pin]

    j2 = part("Connector_Generic", "Conn_01x04", "enc.J2",
              "Connector_JST:JST_XH_B4B-XH-A_1x04_P2.50mm_Vertical", value="STOP",
              mpn="JST B4B-XH-A + XHP-4", ref="J2")
    stop_out, stop_ret, stop_no = net("STOP_NC_OUT"), net("STOP_NC_RET"), net("STOP_NO_RAW")
    stop_out += j2[1]   # NC contact, side 1
    stop_ret += j2[2]   # NC contact, side 2
    stop_no += j2[3]    # NO contact, side 1
    g += j2[4]          # NO contact, side 2


def check_controller_pinout():
    """Compares IDC_PINOUT with the J4 connections in the controller's ui.py."""
    ui = (REPO / "hardware" / "controller" / "schematic" / "ui.py").read_text(encoding="utf-8")
    expected = {
        1: 'raw["ENC_A"] += j4[1]', 3: 'raw["ENC_B"] += j4[3]', 4: 'raw["ENC_SW"] += j4[4]',
        5: "stop_out += j4[5]", 6: "stop_ret += j4[6]", 7: 'raw["STOP_NO"] += j4[7]',
        2: "g += j4[2], j4[8]",
    }
    return [f"J4 pin {pin}: '{line}' not found in ui.py" for pin, line in expected.items()
            if line not in ui]


def main():
    build()
    ERC()
    problems = (erc_logger.error.count + erc_logger.bare_error.count
                + erc_logger.warning.count + erc_logger.bare_warning.count)
    mismatches = check_controller_pinout()
    for m in mismatches:
        print("PINOUT MISMATCH:", m)
    generate_netlist(file_=str(NETLIST))
    print(f"parts: {len(builtins.default_circuit.parts)}, netlist: {NETLIST.relative_to(REPO)}")
    return 1 if problems or mismatches else 0


if __name__ == "__main__":
    sys.exit(main())
