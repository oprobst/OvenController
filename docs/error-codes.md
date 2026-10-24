# Fehlercodes

Zentral definiert in `firmware/lib/diagnostics/include/error_code.h` (`enum class ErrorCode`).
Auf dem Display erscheinen sie als `E` + dreistellige Zahl, im Log als `code=E101`.

**Regel:** Ein vergebener Code wird nie umgedeutet oder wiederverwendet (Constitution,
Prinzip IX). Neue Fehler bekommen neue Nummern.

| Code | Name | Bedeutung | Mögliche Ursache | Abhilfe |
|---|---|---|---|---|
| E101 | `ThermocoupleOpen` | Thermoelement nicht angeschlossen oder Leitung unterbrochen | Stecker lose, Fühler defekt, Leitung gebrochen | Steckverbindung und Fühler prüfen, ggf. Fühler tauschen |
| E102 | `ThermocoupleShortGnd` | Thermoelement-Kurzschluss gegen GND | Isolation beschädigt, Mantel berührt Masse an falscher Stelle | Leitung und Isolation prüfen |
| E103 | `ThermocoupleShortVcc` | Thermoelement-Kurzschluss gegen VCC | Leitung berührt Versorgungsspannung | Verdrahtung prüfen |
| E104 | `TemperatureImplausible` | Messwert außerhalb −20 °C … 350 °C | Falscher Fühlertyp, Störungen, Defekt am Wandler | Fühlertyp (K) und Verdrahtung prüfen |
| E105 | `SensorNoResponse` | Wandler antwortet nicht oder seit 1 s kein gültiger Messwert | MAX31855 nicht bestückt/verdrahtet, Versorgung fehlt, wiederholt wechselnde Fehlmessungen | Verdrahtung und Versorgung des MAX31855 prüfen |

Erkennung und Entprellung: siehe `specs/001-startup-status-menu/data-model.md`.
