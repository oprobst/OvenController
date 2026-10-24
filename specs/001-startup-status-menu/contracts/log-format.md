# Contract: Log-Format (serielle Diagnose)

**Feature**: [../spec.md](../spec.md) | **Prinzip**: IX

UART0, 115200 Baud, 8N1. Eine Zeile pro Ereignis, abgeschlossen mit `\n`, maximal 255 Zeichen
(längere Zeilen werden abgeschnitten und mit `trunc=1` markiert).

## Format

```text
t=<ms> lvl=<LEVEL> mod=<modul> msg="<text>"[ <key>=<value>]*
```

| Feld | Inhalt |
|---|---|
| `t` | ms seit Start, dezimal |
| `lvl` | `ERROR`, `WARN`, `INFO`, `DEBUG` |
| `mod` | Modulname, `[a-z0-9_]+` |
| `msg` | Freitext in Anführungszeichen; `"` und `\` werden mit `\` maskiert |
| weitere | `key=value`, Werte ohne Leerzeichen oder in Anführungszeichen |

## Pflichtereignisse dieses Features (FR-015)

| Ereignis | Beispiel |
|---|---|
| Start | `t=12 lvl=INFO mod=sys msg="boot" fw=0.1.0 pcb=DEV reset=power_on` |
| WLAN verbunden | `t=3810 lvl=INFO mod=wifi msg="connected" rssi=-61` |
| WLAN getrennt | `t=91002 lvl=WARN mod=wifi msg="disconnected" reason=beacon_timeout` |
| WLAN nicht eingerichtet | `t=15 lvl=WARN mod=wifi msg="not configured"` |
| Zeit synchronisiert | `t=4420 lvl=INFO mod=time msg="synced" local="2026-10-24T14:32:05"` |
| Fühlerfehler | `t=5250 lvl=WARN mod=temp msg="sensor fault" code=E101` |
| Fühlerfehler behoben | `t=9750 lvl=INFO mod=temp msg="sensor ok" temp_c=23.25 cj_c=24.06` |
| Menüauswahl | `t=12040 lvl=INFO mod=ui msg="menu select" entry=heat action=none` |
| Abgewiesener UI-Übergang | `t=830 lvl=DEBUG mod=ui msg="transition rejected" screen=welcome event=rotate_cw` |
| Verworfene Logzeilen | `t=60000 lvl=WARN mod=log msg="dropped" count=3` |

## Konfiguration

- Build-Flag `OVEN_LOG_LEVEL`: `0` = aus (`NullLogger`), `1` = ERROR … `4` = DEBUG.
  Standard: `3` (INFO) in `esp32_revdev`.
- Das Abschalten des Logs ändert kein funktionales Verhalten; Fehlercodes bleiben auf dem
  Display sichtbar.
