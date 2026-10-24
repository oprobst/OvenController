# Feature Specification: Startbildschirm mit Statusanzeige und Hauptmenü

**Feature Branch**: `001-startup-status-menu`

**Created**: 2026-10-24

**Status**: Draft

**Input**: User description: "Wir fangen mit der ersten Anforderung an. Der Nutzer soll kurz willkommen geheißen werden wenn das System startet, es soll angezeigt werden ob das system schon im Wifi verbunden ist und - falls ja - die Uhrzeit und das Datum angezeigt bekommen. Die aktuelle Ofentemperatur soll ebenfalls angezeigt werden. Dannach soll ein Menü erscheinen, in dem der Benutzer später auswählen kann, was der Ofen tut."

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Begrüßung und Statusübersicht beim Einschalten (Priority: P1)

Der Benutzer schaltet den Ofen ein. Auf dem Display erscheint kurz eine Begrüßung zusammen mit
Firmware-Version und PCB-Revision. Danach sieht er auf einen Blick, ob der Ofen mit dem WLAN
verbunden ist, bei bestehender Verbindung Uhrzeit und Datum sowie die aktuelle Temperatur im
Ofenraum.

**Why this priority**: Das ist die erste sichtbare Funktion des Geräts. Sie belegt, dass
Controller, Anzeige, Temperaturmessung und Netzwerk zusammenspielen, und zeigt dem Benutzer
sofort, ob der Ofen noch heiß ist.

**Independent Test**: Ofen einschalten und prüfen, dass Begrüßung, Version/Revision,
WLAN-Status, ggf. Uhrzeit/Datum und Ofentemperatur angezeigt werden. Funktioniert ohne Menü.

**Acceptance Scenarios**:

1. **Given** der Ofen ist ausgeschaltet, **When** er eingeschaltet wird, **Then** erscheint
   innerhalb von 2 Sekunden eine Begrüßung mit Firmware-Version und PCB-Revision.
2. **Given** ein bekanntes WLAN ist erreichbar, **When** die Verbindung hergestellt und die
   Uhrzeit bezogen wurde, **Then** zeigt der Statusbereich „WLAN verbunden" sowie Uhrzeit
   (HH:MM) und Datum (TT.MM.JJJJ) in deutscher Ortszeit.
3. **Given** kein WLAN ist erreichbar, **When** der Statusbereich angezeigt wird, **Then**
   steht dort „WLAN nicht verbunden" und es werden weder Uhrzeit noch Datum angezeigt.
4. **Given** der Temperaturfühler liefert gültige Werte, **When** der Statusbereich
   angezeigt wird, **Then** wird die Ofentemperatur in ganzen °C angezeigt und mindestens
   einmal pro Sekunde aktualisiert.

---

### User Story 2 - Übergang ins Hauptmenü (Priority: P2)

Nach der Begrüßung wechselt der Ofen selbstständig in ein Hauptmenü. Dort kann der Benutzer
mit dem Drehencoder zwischen Einträgen wechseln. Die eigentlichen Betriebsarten (Reflow,
Silicagel) werden in späteren Features ergänzt; dieses Feature schafft das Menügerüst.

**Why this priority**: Das Menü ist der Einstiegspunkt für alle späteren Funktionen, liefert
aber allein noch keine Ofenfunktion.

**Independent Test**: Nach dem Start abwarten bzw. Encoder drücken, prüfen, dass das Menü
erscheint, sich durchblättern lässt und die Statuszeile sichtbar bleibt.

**Acceptance Scenarios**:

1. **Given** die Begrüßung wird angezeigt, **When** 3 Sekunden vergangen sind, **Then**
   wechselt die Anzeige automatisch ins Hauptmenü.
2. **Given** die Begrüßung wird angezeigt, **When** der Benutzer den Encoder drückt,
   **Then** wechselt die Anzeige sofort ins Hauptmenü.
3. **Given** das Hauptmenü ist sichtbar, **When** der Benutzer den Encoder dreht, **Then**
   wandert die Markierung zum nächsten bzw. vorherigen Eintrag.
4. **Given** das Hauptmenü ist sichtbar, **When** sich WLAN-Status, Uhrzeit oder Temperatur
   ändern, **Then** wird die Statuszeile ohne Benutzeraktion aktualisiert.
5. **Given** das Hauptmenü zeigt „Heizen", „Profil" und „Setup", **When** der Benutzer
   einen dieser Einträge auswählt, **Then** passiert nichts weiter: das Menü bleibt mit
   derselben Markierung sichtbar und die Heizung bleibt aus.

---

### Edge Cases

- **WLAN verbindet erst nach der Begrüßung**: Der Start wartet nicht auf das WLAN. Die
  Statuszeile im Menü wechselt automatisch auf „verbunden" und zeigt Uhrzeit/Datum, sobald
  diese verfügbar sind.
- **WLAN verbunden, Uhrzeit noch nicht bezogen**: Es wird „WLAN verbunden" angezeigt, an
  Stelle von Uhrzeit/Datum ein Platzhalter (`--:--`), bis eine gültige Zeit vorliegt.
- **WLAN-Verbindung bricht ab**: Die Statuszeile wechselt innerhalb von 10 Sekunden auf
  „WLAN nicht verbunden". Die zuletzt bezogene Uhrzeit läuft intern weiter und wird weiter
  angezeigt, sofern sie einmal gültig synchronisiert wurde.
- **Keine WLAN-Zugangsdaten hinterlegt**: Der Ofen startet normal und zeigt „WLAN nicht
  eingerichtet".
- **Temperaturfühler defekt, nicht angeschlossen oder Messwert unplausibel**: An Stelle der
  Temperatur wird „Fühlerfehler" mit Fehlercode angezeigt. Die Heizung bleibt aus.
- **Ofen beim Einschalten noch heiß**: Die tatsächliche Temperatur wird angezeigt; es wird
  nicht automatisch geheizt oder ein früherer Vorgang fortgesetzt.
- **Neustart durch Watchdog/Brownout während des Betriebs**: Der Ofen durchläuft dieselbe
  Startsequenz wie beim Einschalten und landet im Menü mit Heizung AUS.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: Das System MUSS beim Start eine Begrüßung anzeigen, die Firmware-Version und
  PCB-Revision enthält.
- **FR-002**: Die Begrüßung MUSS nach höchstens 3 Sekunden automatisch ins Hauptmenü
  übergehen und DARF durch Druck auf den Encoder vorzeitig beendet werden.
- **FR-003**: Das System MUSS den WLAN-Status in genau einem von drei Zuständen anzeigen:
  „verbunden", „nicht verbunden" oder „nicht eingerichtet".
- **FR-004**: Bei bestehender WLAN-Verbindung MUSS das System Uhrzeit und Datum aus dem
  Netzwerk beziehen und in deutscher Ortszeit (inkl. Sommer-/Winterzeit) im Format `HH:MM`
  bzw. `TT.MM.JJJJ` anzeigen.
- **FR-005**: Liegt keine gültige Uhrzeit vor, MUSS statt Uhrzeit/Datum ein Platzhalter
  angezeigt werden; es DARF KEINE falsche Zeit (z. B. 01.01.1970) erscheinen.
- **FR-006**: Das System MUSS die aktuelle Ofentemperatur in ganzen Grad Celsius anzeigen und
  mindestens einmal pro Sekunde aktualisieren.
- **FR-007**: Bei einem Fühlerfehler oder unplausiblem Messwert MUSS das System statt einer
  Temperatur „Fühlerfehler" mit dem zugehörigen Fehlercode anzeigen.
- **FR-008**: Der Startvorgang DARF NICHT auf das WLAN warten; Begrüßung und Menü MÜSSEN auch
  ohne Netzwerk erscheinen.
- **FR-009**: Nach der Begrüßung MUSS ein Hauptmenü erscheinen, das mit dem Drehencoder
  bedient wird (Drehen = Eintrag wechseln, Drücken = Eintrag auswählen).
- **FR-010**: Das Hauptmenü MUSS eine dauerhaft sichtbare Statuszeile mit WLAN-Status,
  Uhrzeit (falls verfügbar) und Ofentemperatur enthalten.
- **FR-011**: Das Hauptmenü MUSS die Einträge „Heizen", „Profil" und „Setup" in dieser
  Reihenfolge enthalten. Alle drei MÜSSEN markier- und per Encoder-Druck auswählbar sein; die
  Auswahl löst in diesem Feature keine Aktion aus und das Menü bleibt unverändert sichtbar.
- **FR-012**: Das Menü MUSS so aufgebaut sein, dass spätere Features neue Einträge ergänzen
  können, ohne bestehende Einträge oder die Menüführung zu ändern.
- **FR-013**: Die WLAN-Zugangsdaten (Netzwerkname und Passwort) MÜSSEN beim Bauen der Firmware
  aus einer lokalen, nicht versionierten Konfigurationsdatei übernommen werden. Sie DÜRFEN
  NICHT im Repository landen. Fehlt die Datei oder ist sie leer, gilt das WLAN als „nicht
  eingerichtet".
- **FR-014**: In diesem Feature DARF die Heizung in keinem Zustand eingeschaltet werden; das
  System MUSS nach jedem Start im Zustand „Heizung AUS" sein.
- **FR-015**: Start, WLAN-Statuswechsel, Zeitsynchronisation und Fühlerfehler MÜSSEN über die
  Diagnose-Schnittstelle protokolliert werden.

### Key Entities

- **Systemstatus**: Momentaufnahme der angezeigten Informationen – WLAN-Zustand
  (verbunden / nicht verbunden / nicht eingerichtet), Uhrzeit gültig ja/nein, aktuelle
  Uhrzeit und Datum, Ofentemperatur oder Fühlerfehler mit Code.
- **Menüeintrag**: Ein auswählbarer Punkt im Hauptmenü mit Bezeichnung und zugeordneter
  Aktion (in diesem Feature für alle Einträge: keine Aktion).
- **Anzeigezustand**: Welcher Bildschirm gerade aktiv ist (Begrüßung, Hauptmenü, Fehler).

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Die Begrüßung erscheint in 100 % der Starts innerhalb von 2 Sekunden nach dem
  Einschalten.
- **SC-002**: Das Hauptmenü ist spätestens 5 Sekunden nach dem Einschalten bedienbar – auch
  ohne WLAN.
- **SC-003**: Bei erreichbarem WLAN werden Uhrzeit und Datum innerhalb von 30 Sekunden nach
  dem Einschalten korrekt angezeigt (Abweichung höchstens 1 Minute gegenüber der Ortszeit).
- **SC-004**: Die angezeigte Ofentemperatur weicht bei Raumtemperatur höchstens 3 °C von
  einem Referenzthermometer ab.
- **SC-005**: Ein abgezogener Temperaturfühler wird innerhalb von 2 Sekunden als
  „Fühlerfehler" angezeigt.
- **SC-006**: Ein Benutzer ohne Vorkenntnisse kann beim ersten Einschalten ohne Anleitung
  ablesen, ob der Ofen verbunden ist und wie heiß er ist.

## Assumptions

- Anzeige und Eingabe entsprechen der Vorauswahl in `docs/hardware-umbau.md` (kleines
  OLED/TFT, Drehencoder mit Taster); die endgültige Bauteilwahl erfolgt im Plan.
- Die Ofentemperatur wird über das vorgesehene Thermoelement gemessen; die Plausibilitäts-
  grenzen werden im Plan festgelegt.
- Bedien- und Anzeigesprache ist Deutsch.
- Zeitzone ist fest Mitteleuropa (MEZ/MESZ); eine Zeitzonen-Einstellung ist nicht Teil
  dieses Features.
- Der Ofen hat keine batteriegepufferte Uhr; ohne WLAN steht nach dem Einschalten keine
  Uhrzeit zur Verfügung.
- Heizfunktionen, Profile und die vollständige Sicherheitsüberwachung des Heizbetriebs sind
  nicht Teil dieses Features (eigenes Feature „Sichere Temperaturregelung"). Die Heizung
  bleibt in diesem Feature dauerhaft aus.
- Das Display ist hochkant eingebaut (Bedienfeld 52–56 mm breit, siehe
  `docs/hardware-umbau.md`).
- Die separate Stopp-Taste gehört zum Bedienkonzept, wird in diesem Feature aber noch nicht
  ausgewertet; es gibt noch nichts zu stoppen. Ebenso folgt „langer Druck = Zurück" erst mit
  dem ersten Untermenü.
- Der Buzzer wird in diesem Feature nicht verwendet.
- Das Ändern der WLAN-Zugangsdaten über das Menü („Setup") folgt in einem späteren Feature;
  bis dahin erfordert ein WLAN-Wechsel eine neu gebaute Firmware.

## Clarifications

### Session 2026-10-24

- Q: Welche Einträge soll das Menü in diesem Feature enthalten? → A: Platzhalter „Heizen",
  „Profil", „Setup"; alle auswählbar, ohne Wirkung.
- Q: Wie gelangen die WLAN-Zugangsdaten in den Ofen? → A: Fest beim Bauen der Firmware aus
  einer lokalen Datei; Ändern im Menü folgt als späteres Feature.
