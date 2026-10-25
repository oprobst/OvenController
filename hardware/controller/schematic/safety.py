"""Block safety (T014): enable flip-flop, clear line CLR, external watchdog, PCB thermostat.

research.md R6/R7, data-model.md (Heizungsfreigabe). CLR is active LOW and is pulled to GND by
100k; 3.3 V reaches it only through the stop button's normally-closed contact (cable broken =
stop). Every clear source pulls CLR LOW against the 4.7k series resistor.
"""

from common import nc, capacitor, gnd, net, part, resistor, testpoint, v3v3

# SN74LVC74A (KiCad symbol 74HC74, same pinout, see hardware/lib/README.md)
CLR1, D1, CLK1, PRE1, Q1, QN1, GND_PIN, QN2, Q2, PRE2, CLK2, D2, CLR2, VCC = range(1, 15)


def build():
    g, p3v3 = gnd(), v3v3()
    clr, arm, latch_q = net("CLR"), net("ARM"), net("LATCH_Q")
    disarm, wdi, clr_sense = net("DISARM"), net("WDI"), net("CLR_SENSE")
    stop_out, stop_ret = net("STOP_NC_OUT"), net("STOP_NC_RET")

    ff = part("74xx", "74HC74", "saf.U1", "Package_SO:SOIC-14_3.9x8.7mm_P1.27mm",
              value="SN74LVC74AD", mpn="TI SN74LVC74AD", source="Mouser")
    clr += ff[CLR1]
    p3v3 += ff[D1], ff[PRE1], ff[VCC], ff[CLR2], ff[PRE2]
    arm += ff[CLK1]
    latch_q += ff[Q1]
    ff[QN1] += nc()
    g += ff[GND_PIN], ff[D2], ff[CLK2]  # unused second flip-flop with fixed inputs
    ff[Q2] += nc()
    ff[QN2] += nc()
    capacitor("saf.C1", "100n", p3v3, g)

    resistor("saf.R1", "100k", arm, g)        # ARM low while the MCU is in reset
    resistor("saf.R2", "100k", latch_q, g)    # defined level before the flip-flop is powered

    # Clear line: pull-down, supply only via the stop button's NC contact.
    resistor("saf.R3", "100k", clr, g)
    capacitor("saf.C2", "1n", clr, g)
    resistor("saf.R4", "4.7k", stop_ret, clr)
    resistor("saf.R5", "220", p3v3, stop_out)  # limits current if the cable is shorted
    resistor("saf.R6", "10k", clr, clr_sense)

    # External watchdog / supervisor: open-drain ~RESET on CLR.
    wd = part("oven", "TPS3828-33DBV", "saf.U2", "Package_TO_SOT_SMD:SOT-23-5",
              value="TPS3828-33DBV", mpn="TI TPS3828-33DBVR", source="Mouser")
    clr += wd[1]       # ~RESET
    g += wd[2]
    disarm += wd[3]    # ~MR, internal 90k pull-up
    wdi += wd[4]
    p3v3 += wd[5]
    resistor("saf.R7", "2.2k", wdi, g)  # mandatory: floating WDI disables the watchdog
    capacitor("saf.C3", "100n", p3v3, g)

    # PCB over-temperature switch 65 °C, hysteresis 10 °C (HYST = VCC).
    ts = part("oven", "TC6501", "saf.U3", "Package_TO_SOT_SMD:SOT-23-5",
              value="TC6501P065", mpn="Microchip TC6501P065VCTTR", source="DigiKey 412996")
    g += ts[1], ts[2]
    p3v3 += ts[3], ts[4]
    clr += ts[5]
    capacitor("saf.C4", "100n", p3v3, g)

    testpoint("saf.TP1", latch_q)
    testpoint("saf.TP2", clr)
    testpoint("saf.TP3", arm)
    testpoint("saf.TP4", wdi)
