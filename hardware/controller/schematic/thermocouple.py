"""Block thermocouple (T013): two MAX31856 channels (top, bottom) with Omega PCC-SMP sockets.

research.md R4/R6. Both chips share SPI3; each FAULT output joins the clear line CLR through a
Schottky diode (cathode at FAULT), so open-drain and push-pull outputs both work.
"""

from skidl import Pin

from common import nc, FP_C_0805, FP_SCHOTTKY_SOD123, capacitor, gnd, net, part, resistor, schottky, v3v3

# MAX31856 TSSOP-14 pins (KiCad symbol, checked in hardware/lib/README.md)
AGND, BIAS, T_MINUS, T_PLUS, AVDD, DNC, DRDY, DVDD, CS, SCK, SDO, SDI, FAULT, DGND = range(1, 15)


def channel(name, socket_ref):
    g, p3v3 = gnd(), v3v3()
    tc_p, tc_n = net(f"TC_{name}_P"), net(f"TC_{name}_N")
    cs, fault = net(f"TC_CS_{name}"), net(f"FAULT_{name}")
    miso, sck, mosi = net("TC_MISO"), net("TC_SCK"), net("TC_MOSI")
    tag = name.lower()

    u = part("Sensor_Temperature", "MAX31856", f"tc.{tag}.U1",
             "Package_SO:TSSOP-14_4.4x5mm_P0.65mm", value="MAX31856MUD+",
             mpn="Analog Devices MAX31856MUD+", source="Mouser")
    g += u[AGND], u[DGND]
    p3v3 += u[AVDD], u[DVDD]
    tc_p += u[T_PLUS]
    tc_n += u[T_MINUS], u[BIAS]  # BIAS supplies the common-mode voltage on T-
    cs += u[CS]
    sck += u[SCK]
    u[SDO].func = Pin.types.TRISTATE  # SDO is high-impedance while CS is inactive
    miso += u[SDO]
    mosi += u[SDI]
    fault += u[FAULT]
    u[DRDY] += nc()
    u[DNC] += nc()

    capacitor(f"tc.{tag}.C1", "100n", p3v3, g)   # AVDD
    capacitor(f"tc.{tag}.C2", "100n", p3v3, g)   # DVDD
    capacitor(f"tc.{tag}.C3", "10n", tc_p, tc_n)  # differential input filter
    resistor(f"tc.{tag}.R1", "10k", p3v3, cs)     # CS inactive while the MCU boots

    clr = net("CLR")
    schottky(f"tc.{tag}.D1", "MBR0530", clr, fault, FP_SCHOTTKY_SOD123)

    j = part("Connector_Generic", "Conn_01x02", f"tc.{tag}.J1", "oven:Omega_PCC-SMP_Horizontal",
             value=f"TC {name.lower()}", mpn="Omega PCC-SMP-K", source="Farnell 3794917",
             ref=socket_ref)
    tc_p += j[1]
    tc_n += j[2]
    return u


def build():
    channel("TOP", "J5")
    channel("BOT", "J6")
    capacitor("tc.C1", "10u", v3v3(), gnd(), FP_C_0805)
