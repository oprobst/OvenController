# Contracts: Steckverbinder, Pinbelegung und Testpunkte (REV A)

Hardware-Schnittstelle zwischen Platine, Lastteil, Bedienteil und Gehäuse (Prinzip VI). Jede
Änderung hier muss in Firmware (`board_rev_a.h`), KiCad-Projekt und Doku nachgezogen werden.
Pin 1 ist auf der Platine markiert.

## J1 – Netz (230 V)

Steckbare Schraubklemme 2-polig, Raster 5,08 mm. Nur im Netzbereich, Beschriftung „230 V~".

| Pin | Signal |
|---|---|
| 1 | L (nach Hauptschalter und Gerätesicherung der Kaltgerätebuchse) |
| 2 | N |

Kein PE an der Platine.

## J2 – Lastteil (Kleinspannung)

Steckbare Schraubklemme 6-polig, Raster 3,81 mm.

| Pin | Signal | Gegenstelle |
|---|---|---|
| 1 | +5V_SSR (geschaltet durch Freigabe) | „+" beider SSR-Eingänge |
| 2 | SSR_TOP− | „−" SSR Oberhitze |
| 3 | SSR_BOT− | „−" SSR Unterhitze |
| 4 | +5V (dauernd) | Relaisspule A1 |
| 5 | RELAY− | Relaisspule A2 |
| 6 | GND | Reserve / Schirm |

## J3 – Display

IDC 2×5, Raster 2,54 mm, Flachbandkabel ≤ 20 cm.

| Pin | Signal | Pin | Signal |
|---|---|---|---|
| 1 | 3V3 | 2 | GND |
| 3 | TFT_SCK | 4 | GND |
| 5 | TFT_MOSI | 6 | GND |
| 7 | TFT_CS | 8 | TFT_DC |
| 9 | TFT_RST | 10 | TFT_BL (geschaltete 3,3 V) |

## J4 – Bedienteil (Encoder-Träger und Stopp-Taste)

IDC 2×4, Raster 2,54 mm. Gegenstück gleicher Belegung auf dem Encoder-Träger.

| Pin | Signal | Pin | Signal |
|---|---|---|---|
| 1 | ENC_A | 2 | GND |
| 3 | ENC_B | 4 | ENC_SW |
| 5 | STOP_NC_OUT (3,3 V zum Öffner) | 6 | STOP_NC_RET (vom Öffner zur Löschleitung) |
| 7 | STOP_NO (Schließer → GPIO) | 8 | GND |

Auf dem Encoder-Träger: Klemme für die Stopp-Taste (Öffner: 2 Litzen, Schließer: 2 Litzen,
Schließer gegen GND).

## J5, J6 – Thermoelemente

Mini-Thermoelement-Buchse Typ K, Print, gelb. J5 = oben, J6 = unten, Polarität (+/−) im
Bestückungsdruck.

## J7 – USB-C

An der Platinenkante; Panel-Verlängerung zur Gehäusewand.

## J8 – Antenne

U.FL am Modul (Modulkante), Pigtail zur RP-SMA-Durchführung.

## J9 – Debug-UART (optional bestückt)

Pinleiste 3-polig: 1 GND, 2 TX (GPIO 43), 3 RX (GPIO 44).

## Pinbelegung ESP32-S3 (REV A)

| Signal | GPIO | Richtung | Beschaltung |
|---|---|---|---|
| REV_STRAP | 1 | ADC | Teiler 10 kΩ / 10 kΩ (1 %) |
| LED_DIAG | 2 | Aus | LED + Vorwiderstand nach GND |
| HEAT_TOP | 4 | Aus | IRLML6346, 100 kΩ Pull-down |
| HEAT_BOT | 5 | Aus | IRLML6346, 100 kΩ Pull-down |
| ARM | 6 | Aus | CLK des Flipflops, 100 kΩ Pull-down |
| TFT_BL | 7 | Aus (PWM) | AO3401A, 100 kΩ Pull-up am Gate |
| TFT_RST | 8 | Aus | |
| TFT_DC | 9 | Aus | |
| TFT_CS | 10 | Aus | 10 kΩ Pull-up |
| TFT_MOSI | 11 | Aus | 33 Ω Serie |
| TFT_SCK | 12 | Aus | 33 Ω Serie |
| TC_MISO | 13 | Ein | |
| TC_SCK | 14 | Aus | |
| TC_MOSI | 15 | Aus | |
| TC_CS_TOP | 16 | Aus | 10 kΩ Pull-up |
| TC_CS_BOT | 17 | Aus | 10 kΩ Pull-up |
| DISARM | 18 | Aus | an /MR TPS3828 (interner Pull-up) |
| USB_D− / USB_D+ | 19 / 20 | USB | USBLC6-2SC6 |
| WDI | 21 | Aus | TPS3828 WDI (Pull-down nach Prüfung R7) |
| BUZZER | 35 | Aus (PWM) | IRLML6346, 100 kΩ Pull-down |
| CLR_SENSE | 36 | Ein | 10 kΩ Serie von der Löschleitung |
| ENC_A / ENC_B / ENC_SW | 39 / 40 / 41 | Ein | 10 kΩ Pull-up, 10 kΩ Serie, 10 nF |
| STOP_NO | 42 | Ein | 10 kΩ Pull-up, 10 kΩ Serie, 10 nF |
| LATCH_Q | 47 | Ein | |
| UART TX / RX | 43 / 44 | – | J9 |
| BOOT | 0 | Ein | Taster nach GND |
| frei | 3, 37, 38, 45, 46, 48 | – | 3, 45, 46 Strapping: nicht beschalten |

## Testpunkte

TP1 5V, TP2 3V3, TP3 und TP11 GND, TP4 LATCH_Q, TP5 CLR, TP6 ARM, TP7 HEAT_TOP, TP8 HEAT_BOT,
TP9 +5V_SSR, TP10 WDI. Keine Testpunkte im Netzbereich. Steckverbinder und Testpunkte tragen im
Schaltplan genau diese Referenzen (J1–J7, J9, TP1–TP11); die übrigen Referenzen vergibt SKiDL.

## Mechanik

- Außenmaße ≤ 150 × 150 mm, Ziel ≤ 100 × 100 mm; Bauteilhöhe ≤ 40 mm (höchstes Teil
  IRM-10-5).
- 4 Montagelöcher M3 ohne Kupfer, Lage wird mit dem Feature „Controller-Gehäuse" abgestimmt
  (Übergabe als STEP und Maßzeichnung).
