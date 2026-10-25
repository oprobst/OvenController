"""Block mcu (T011): ESP32-S3-WROOM-1U-N8, reset/boot, diagnostic LED, revision strap, debug UART.

research.md R3, R11, R12. GPIO assignment: contracts/connectors.md (Pinbelegung ESP32-S3).
"""

from common import (
    nc,
    FP_C_0805,
    capacitor,
    electrolytic,
    gnd,
    net,
    part,
    resistor,
    v3v3,
)

# Module pin numbers per Espressif ESP32-S3-WROOM-1/1U datasheet v1.8, table 3-1
# (checked against the KiCad symbol, hardware/lib/README.md).
MODULE_PIN = {
    "GND": 1, "3V3": 2, "EN": 3,
    4: 4, 5: 5, 6: 6, 7: 7, 15: 8, 16: 9, 17: 10, 18: 11, 8: 12, 19: 13, 20: 14, 3: 15,
    46: 16, 9: 17, 10: 18, 11: 19, 12: 20, 13: 21, 14: 22, 21: 23, 47: 24, 48: 25, 45: 26,
    0: 27, 35: 28, 36: 29, 37: 30, 38: 31, 39: 32, 40: 33, 41: 34, 42: 35, 44: 36, 43: 37,
    2: 38, 1: 39,
}

# GPIO -> net name (connectors.md). USB D-/D+ (19/20) are connected in usb.py.
GPIO_NETS = {
    1: "REV_STRAP", 2: "LED_DIAG", 4: "HEAT_TOP", 5: "HEAT_BOT", 6: "ARM",
    7: "TFT_BL_PWM", 8: "TFT_RST", 9: "TFT_DC", 10: "TFT_CS", 11: "TFT_MOSI_MCU",
    12: "TFT_SCK_MCU", 13: "TC_MISO", 14: "TC_SCK", 15: "TC_MOSI", 16: "TC_CS_TOP",
    17: "TC_CS_BOT", 18: "DISARM", 19: "USB_DN", 20: "USB_DP", 21: "WDI", 35: "BUZZER",
    36: "CLR_SENSE", 39: "ENC_A", 40: "ENC_B", 41: "ENC_SW", 42: "STOP_NO", 47: "LATCH_Q",
    43: "UART_TX", 44: "UART_RX", 0: "BOOT",
}
UNUSED_GPIOS = (3, 37, 38, 45, 46, 48)  # 3, 45, 46 are strapping pins: leave unconnected


def build():
    g, p3v3 = gnd(), v3v3()
    en, boot = net("EN"), net("BOOT")

    u = part("RF_Module", "ESP32-S3-WROOM-1", "mcu.U1", "RF_Module:ESP32-S3-WROOM-1U",
             value="ESP32-S3-WROOM-1U-N8", mpn="Espressif ESP32-S3-WROOM-1U-N8",
             source="Benutzer")
    p3v3 += u[MODULE_PIN["3V3"]]
    g += u[1], u[40], u[41]
    en += u[MODULE_PIN["EN"]]
    for gpio, name in GPIO_NETS.items():
        n = net(name)
        n += u[MODULE_PIN[gpio]]
    for gpio in UNUSED_GPIOS:
        u[MODULE_PIN[gpio]] += nc()

    # Supply decoupling at the module; bulk for WiFi peaks (research R3).
    capacitor("mcu.C1", "10u", p3v3, g, FP_C_0805)
    capacitor("mcu.C2", "100n", p3v3, g)
    electrolytic("mcu.C3", "100u/16V", p3v3, g)

    # EN: RC per Espressif hardware design guidelines, RESET button to GND.
    resistor("mcu.R1", "10k", p3v3, en)
    capacitor("mcu.C4", "1u", en, g)
    sw_rst = part("Switch", "SW_Push", "mcu.SW1", "Button_Switch_THT:SW_PUSH_6mm",
                  value="RESET")
    en += sw_rst[1]
    g += sw_rst[2]

    # GPIO0 (strapping): HIGH = boot from flash; BOOT button pulls it LOW.
    resistor("mcu.R2", "10k", p3v3, boot)
    sw_boot = part("Switch", "SW_Push", "mcu.SW2", "Button_Switch_THT:SW_PUSH_6mm",
                   value="BOOT")
    boot += sw_boot[1]
    g += sw_boot[2]

    # Diagnostic LED, active HIGH (research R11).
    led_a = net("LED_DIAG_A")
    resistor("mcu.R3", "1k", net("LED_DIAG"), led_a)
    led = part("Device", "LED", "mcu.D1", "LED_SMD:LED_0805_2012Metric", value="green")
    g += led["K"]
    led_a += led["A"]

    # Revision strap REV A: 10k/10k 1 % -> 1.65 V at GPIO 1 (research R12).
    strap = net("REV_STRAP")
    resistor("mcu.R4", "10k 1%", p3v3, strap)
    resistor("mcu.R5", "10k 1%", strap, g)
    capacitor("mcu.C5", "100n", strap, g)

    # Debug UART, optional population.
    j9 = part("Connector_Generic", "Conn_01x03", "mcu.J9",
              "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical", value="UART",
              ref="J9")
    g += j9[1]
    tx, rx = net("UART_TX"), net("UART_RX")
    tx += j9[2]
    rx += j9[3]
