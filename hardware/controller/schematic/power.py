"""Block power (T010): mains input, IRM-10-5, diode-OR of PSU and USB, 3.3 V LDO.

research.md R2 (mains, isolation) and R14 (5 V / 3.3 V paths).
Nets L_IN, N_IN, L_FUSED and +5V_PSU belong to net class Mains/primary or the PSU output.
"""

from common import (
    FP_C_0805,
    FP_SCHOTTKY_SMA,
    capacitor,
    electrolytic,
    gnd,
    net,
    part,
    schottky,
    testpoint,
    v3v3,
    v5,
)


def build():
    # Mains nets are driven from outside the board (J1); marked as power for ERC.
    l_in, n_in = net("L_IN", power=True), net("N_IN", power=True)
    l_fused = net("L_FUSED", power=True)
    v5_psu, vbus = net("+5V_PSU"), net("VBUS")
    g, p3v3, p5v = gnd(), v3v3(), v5()

    j1 = part(
        "Connector_Generic", "Conn_01x02", "pwr.J1",
        "Connector_Phoenix_MSTB:PhoenixContact_MSTBA_2,5_2-G-5,08_1x02_P5.08mm_Horizontal",
        value="230V~", mpn="Phoenix MSTBA 2,5/2-G-5,08 + MSTB 2,5/2-ST-5,08",
    )
    l_in += j1[1]
    n_in += j1[2]

    f1 = part(
        "Device", "Fuse", "pwr.F1",
        "Fuse:Fuseholder_Cylinder-5x20mm_Schurter_FAB_0031-355x_Horizontal_Closed",
        value="T500mA", mpn="Schurter FAB 0031.355x + 5x20 T500mA",
    )
    l_in += f1[1]
    l_fused += f1[2]

    rv1 = part("Device", "Varistor", "pwr.RV1", "Varistor:RV_Disc_D7mm_W3.4mm_P5mm",
               value="S07K275", mpn="TDK B72207S2271K101")
    l_fused += rv1[1]
    n_in += rv1[2]

    ps1 = part("Converter_ACDC", "IRM-10-5", "pwr.PS1",
               "Converter_ACDC:Converter_ACDC_MeanWell_IRM-10-xx_THT",
               value="IRM-10-5", mpn="Mean Well IRM-10-5", source="Reichelt IRM-10-5")
    n_in += ps1[1]       # AC/N
    l_fused += ps1[2]    # AC/L
    g += ps1[3]      # -Vo
    v5_psu += ps1[4]     # +Vo

    # Diode-OR: neither the PSU nor USB can feed the other (research R14).
    schottky("pwr.D1", "SS34", v5_psu, p5v, FP_SCHOTTKY_SMA)
    schottky("pwr.D2", "SS34", vbus, p5v, FP_SCHOTTKY_SMA)
    capacitor("pwr.C1", "10u", v5_psu, g, FP_C_0805)
    electrolytic("pwr.C2", "100u/16V", p5v, g)

    u1 = part("Regulator_Linear", "AP7361C-33E", "pwr.U1",
              "Package_TO_SOT_SMD:SOT-223-3_TabPin2", value="AP7361C-33E",
              mpn="Diodes AP7361C-33E-13", source="Mouser")
    p5v += u1[1]        # IN
    g += u1[2]       # GND (tab)
    p3v3 += u1[3]      # OUT
    capacitor("pwr.C3", "10u", p5v, g, FP_C_0805)
    capacitor("pwr.C4", "10u", p3v3, g, FP_C_0805)
    capacitor("pwr.C5", "100n", p3v3, g)

    testpoint("pwr.TP1", p5v)
    testpoint("pwr.TP2", p3v3)
    testpoint("pwr.TP3", g)
    testpoint("pwr.TP4", g)
