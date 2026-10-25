"""Shared setup for the controller schematic: SKiDL configuration, nets and part helpers.

Every block module imports from here so that nets with the same name are the same object and
all parts carry a stable tag (keeps references and footprints stable in KiCad across runs).
Values and choices: specs/002-controller-pcb/research.md and contracts/connectors.md.
"""

import builtins
import os
from pathlib import Path

KICAD_SHARE = Path(r"D:\hobby\Elektronik\KiCad\10.0\share\kicad")
os.environ.setdefault("KICAD10_SYMBOL_DIR", str(KICAD_SHARE / "symbols"))
os.environ.setdefault("KICAD10_FOOTPRINT_DIR", str(KICAD_SHARE / "footprints"))

from skidl import KICAD10, POWER, Net, Part, lib_search_paths, set_default_tool  # noqa: E402

set_default_tool(KICAD10)
LIB_DIR = Path(__file__).resolve().parents[2] / "lib"
if str(LIB_DIR) not in lib_search_paths[KICAD10]:
    lib_search_paths[KICAD10].append(str(LIB_DIR))

# Footprints used by several blocks
FP_R = "Resistor_SMD:R_0603_1608Metric"
FP_C = "Capacitor_SMD:C_0603_1608Metric"
FP_C_0805 = "Capacitor_SMD:C_0805_2012Metric"
FP_CP = "Capacitor_THT:CP_Radial_D6.3mm_P2.50mm"
FP_SOT23 = "Package_TO_SOT_SMD:SOT-23"
FP_SCHOTTKY_SMA = "Diode_SMD:D_SMA"
FP_SCHOTTKY_SOD123 = "Diode_SMD:D_SOD-123"
FP_TESTPOINT = "TestPoint:TestPoint_Keystone_5000-5004_Miniature"

_nets = {}


def nc():
    """The no-connect net of the current circuit (marks pins as intentionally unconnected)."""
    return builtins.NC  # SKiDL 3 provides NC (and default_circuit) only as builtins


def net(name, power=False):
    """Returns the net with this name, creating it once. Power nets are marked as driven for ERC."""
    if name not in _nets:
        n = Net(name)
        if power:
            n.drive = POWER
        _nets[name] = n
    return _nets[name]


def gnd():
    return net("GND", power=True)


def v3v3():
    return net("+3V3", power=True)


def v5():
    return net("+5V", power=True)


def part(lib, name, tag, footprint, value=None, mpn=None, source=None, dnp=False):
    """Creates a part with a stable tag and BOM fields (MPN, source, DNP)."""
    p = Part(lib, name, footprint=footprint, tag=tag)
    if value is not None:
        p.value = value
    if mpn:
        p.fields["MPN"] = mpn
    if source:
        p.fields["Source"] = source
    if dnp:
        p.fields["DNP"] = "yes"
    return p


def resistor(tag, value, a, b, footprint=FP_R, dnp=False):
    """Resistor between nets a and b. Values from the inventory unless noted otherwise."""
    r = part("Device", "R", tag, footprint, value=value, source="Magazin", dnp=dnp)
    a += r[1]
    b += r[2]
    return r


def capacitor(tag, value, a, b, footprint=FP_C):
    c = part("Device", "C", tag, footprint, value=value, source="Magazin")
    a += c[1]
    b += c[2]
    return c


def electrolytic(tag, value, plus, minus):
    c = part("Device", "C_Polarized", tag, FP_CP, value=value, mpn="105 °C type")
    plus += c[1]
    minus += c[2]
    return c


def schottky(tag, value, anode, cathode, footprint):
    d = part("Device", "D_Schottky", tag, footprint, value=value, source="Magazin")
    cathode += d["K"]
    anode += d["A"]
    return d


def nmos(tag, value, gate, source, drain):
    """N-channel MOSFET in SOT-23 (1 G, 2 S, 3 D), e.g. IRLML6346 or BSS138."""
    q = part("Transistor_FET", "AO3400A", tag, FP_SOT23, value=value, mpn=value, source="Magazin")
    gate += q[1]
    source += q[2]
    drain += q[3]
    return q


def pmos(tag, value, gate, source, drain):
    """P-channel MOSFET in SOT-23 (1 G, 2 S, 3 D), AO3401A."""
    q = part("Transistor_FET", "AO3401A", tag, FP_SOT23, value=value, mpn=value, source="Magazin")
    gate += q[1]
    source += q[2]
    drain += q[3]
    return q


def testpoint(tag, signal):
    tp = part("Connector", "TestPoint", tag, FP_TESTPOINT, value=signal.name)
    signal += tp[1]
    return tp
