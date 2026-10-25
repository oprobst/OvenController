"""Block usb (T012): USB-C receptacle at the native USB of the ESP32-S3.

research.md R3. CC pull-downs 5.1k (device), ESD array, shield via population option.
"""

from common import nc, capacitor, gnd, net, part, resistor


def build():
    g = gnd()
    vbus, dp, dn = net("VBUS"), net("USB_DP"), net("USB_DN")
    cc1, cc2, shield = net("USB_CC1"), net("USB_CC2"), net("USB_SHIELD")

    j7 = part("Connector", "USB_C_Receptacle_USB2.0_16P", "usb.J7",
              "Connector_USB:USB_C_Receptacle_GCT_USB4085", value="USB-C",
              mpn="GCT USB4085-GF-A", source="Mouser")
    vbus += j7["A4"], j7["A9"], j7["B4"], j7["B9"]
    g += j7["A1"], j7["A12"], j7["B1"], j7["B12"]
    cc1 += j7["A5"]
    cc2 += j7["B5"]
    dp += j7["A6"], j7["B6"]
    dn += j7["A7"], j7["B7"]
    j7["A8"] += nc()
    j7["B8"] += nc()
    shield += j7["SH"]

    resistor("usb.R1", "5.1k", cc1, g)
    resistor("usb.R2", "5.1k", cc2, g)

    # Shield: either 1M || 4.7n (default) or 0 R (option, not populated) to GND.
    resistor("usb.R3", "1M", shield, g)
    capacitor("usb.C1", "4.7n", shield, g)
    resistor("usb.R4", "0R", shield, g, dnp=True)

    # ESD protection: line 1 (pins 1/6) = D-, line 2 (pins 3/4) = D+.
    u = part("Power_Protection", "USBLC6-2SC6", "usb.U1", "Package_TO_SOT_SMD:SOT-23-6",
             value="USBLC6-2SC6", mpn="ST USBLC6-2SC6", source="Magazin")
    dn += u[1], u[6]
    dp += u[3], u[4]
    g += u[2]
    vbus += u[5]
