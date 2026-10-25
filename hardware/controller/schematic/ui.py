"""Block ui (T016): display connector J3 with backlight switch, control panel connector J4 with
input filters and ESD protection, buzzer driver.

research.md R9-R11, contracts/connectors.md (J3, J4).
"""

from common import (
    FP_SCHOTTKY_SOD123,
    capacitor,
    gnd,
    net,
    nmos,
    part,
    pmos,
    resistor,
    schottky,
    v3v3,
    v5,
)


def input_filter(tag, raw, mcu):
    """10k pull-up on the cable side, 10k series, 10n to GND at the MCU (Alps EC11E datasheet)."""
    resistor(f"{tag}.R1", "10k", v3v3(), raw)
    resistor(f"{tag}.R2", "10k", raw, mcu)
    capacitor(f"{tag}.C1", "10n", mcu, gnd())


def esd_pair(tag, line1, line2):
    """USBLC6-2SC6 used as two-line ESD clamp to +3V3 (line 1: pins 1/6, line 2: pins 3/4)."""
    g, p3v3 = gnd(), v3v3()
    u = part("Power_Protection", "USBLC6-2SC6", tag, "Package_TO_SOT_SMD:SOT-23-6",
             value="USBLC6-2SC6", mpn="ST USBLC6-2SC6", source="Magazin")
    line1 += u[1], u[6]
    line2 += u[3], u[4]
    g += u[2]
    p3v3 += u[5]


def build():
    g, p3v3, p5v = gnd(), v3v3(), v5()

    # Display (J3): series resistors at the MCU, CS pull-up, switched backlight.
    sck, sck_mcu = net("TFT_SCK"), net("TFT_SCK_MCU")
    mosi, mosi_mcu = net("TFT_MOSI"), net("TFT_MOSI_MCU")
    cs, dc, rst = net("TFT_CS"), net("TFT_DC"), net("TFT_RST")
    bl, bl_pwm = net("TFT_BL"), net("TFT_BL_PWM")
    resistor("ui.R1", "33", sck_mcu, sck)
    resistor("ui.R2", "33", mosi_mcu, mosi)
    resistor("ui.R3", "10k", p3v3, cs)
    pmos("ui.Q1", "AO3401A", bl_pwm, p3v3, bl)
    resistor("ui.R4", "100k", p3v3, bl_pwm)  # backlight off while the MCU boots

    j3 = part("Connector_Generic", "Conn_02x05_Odd_Even", "ui.J3",
              "Connector_IDC:IDC-Header_2x05_P2.54mm_Vertical", value="DISPLAY",
              mpn="Wannenstecker 2x5 RM2.54")
    p3v3 += j3[1]
    g += j3[2], j3[4], j3[6]
    sck += j3[3]
    mosi += j3[5]
    cs += j3[7]
    dc += j3[8]
    rst += j3[9]
    bl += j3[10]

    # Control panel (J4): encoder and stop button.
    raw = {n: net(f"{n}_RAW") for n in ("ENC_A", "ENC_B", "ENC_SW", "STOP_NO")}
    stop_out, stop_ret = net("STOP_NC_OUT"), net("STOP_NC_RET")
    j4 = part("Connector_Generic", "Conn_02x04_Odd_Even", "ui.J4",
              "Connector_IDC:IDC-Header_2x04_P2.54mm_Vertical", value="PANEL",
              mpn="Wannenstecker 2x4 RM2.54")
    raw["ENC_A"] += j4[1]
    g += j4[2], j4[8]
    raw["ENC_B"] += j4[3]
    raw["ENC_SW"] += j4[4]
    stop_out += j4[5]
    stop_ret += j4[6]
    raw["STOP_NO"] += j4[7]

    for name in ("ENC_A", "ENC_B", "ENC_SW", "STOP_NO"):
        input_filter(f"ui.{name.lower()}", raw[name], net(name))
    esd_pair("ui.U1", raw["ENC_A"], raw["ENC_B"])
    esd_pair("ui.U2", raw["ENC_SW"], raw["STOP_NO"])
    esd_pair("ui.U3", stop_out, stop_ret)

    # Passive magnetic buzzer from +5V, low-side switch, freewheeling diode.
    buzzer, buzzer_n, buzzer_g = net("BUZZER"), net("BUZZER_N"), net("BUZZER_G")
    bz = part("Device", "Buzzer", "ui.BZ1", "Buzzer_Beeper:Buzzer_12x9.5RM7.6",
              value="Buzzer 5V", mpn="passive magnetic 5 V, >= 70 °C (tbd)")
    p5v += bz[1]
    buzzer_n += bz[2]
    resistor("ui.R5", "100", buzzer, buzzer_g)
    resistor("ui.R6", "100k", buzzer_g, g)
    nmos("ui.Q2", "IRLML6346", buzzer_g, g, buzzer_n)
    schottky("ui.D1", "MBR0530", buzzer_n, p5v, FP_SCHOTTKY_SOD123)
