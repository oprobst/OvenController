"""Block load (T015): SSR supply switched by the enable flip-flop, low-side switches per heater,
safety relay driver, load connector J2.

research.md R8. A heater circuit only conducts if LATCH_Q (high side) AND its heat command
(low side) are present (FR-019).
"""

from common import (
    FP_SCHOTTKY_SOD123,
    gnd,
    net,
    nmos,
    part,
    pmos,
    resistor,
    schottky,
    testpoint,
    v5,
)


def low_side(tag, command, load_neg):
    """IRLML6346 switching load_neg to GND; gate from command via 100R, 100k pull-down."""
    g = gnd()
    gate = net(f"{command.name}_G")
    resistor(f"{tag}.R1", "100", command, gate)
    resistor(f"{tag}.R2", "100k", gate, g)
    nmos(f"{tag}.Q1", "IRLML6346", gate, g, load_neg)


def build():
    g, p5v = gnd(), v5()
    latch_q, ssr_en_n, v5_ssr = net("LATCH_Q"), net("SSR_EN_N"), net("+5V_SSR")
    heat_top, heat_bot = net("HEAT_TOP"), net("HEAT_BOT")
    ssr_top_n, ssr_bot_n, relay_n = net("SSR_TOP_N"), net("SSR_BOT_N"), net("RELAY_N")

    # High side: AO3401A from +5V to +5V_SSR; gate pulled to GND by BSS138 when LATCH_Q is HIGH.
    pmos("load.Q1", "AO3401A", ssr_en_n, p5v, v5_ssr)
    resistor("load.R1", "100k", p5v, ssr_en_n)
    nmos("load.Q2", "BSS138", latch_q, g, ssr_en_n)

    low_side("load.top", heat_top, ssr_top_n)
    low_side("load.bot", heat_bot, ssr_bot_n)

    # Safety relay coil (Finder 40.61.9.005, 130 mA) switched by LATCH_Q.
    low_side("load.relay", latch_q, relay_n)
    schottky("load.D1", "MBR0530", relay_n, p5v, FP_SCHOTTKY_SOD123)

    j2 = part("Connector_Generic", "Conn_01x06", "load.J2",
              "Connector_Phoenix_MC:PhoenixContact_MC_1,5_6-G-3.81_1x06_P3.81mm_Horizontal",
              value="LOAD", mpn="Phoenix MC 1,5/6-G-3,81 + MC 1,5/6-ST-3,81", ref="J2")
    v5_ssr += j2[1]
    ssr_top_n += j2[2]
    ssr_bot_n += j2[3]
    p5v += j2[4]
    relay_n += j2[5]
    g += j2[6]

    testpoint("load.TP1", heat_top, "TP7")
    testpoint("load.TP2", heat_bot, "TP8")
    testpoint("load.TP3", v5_ssr, "TP9")
