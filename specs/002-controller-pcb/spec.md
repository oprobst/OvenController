# Feature Specification: Controller-Platine (PCB REV A)

**Feature Branch**: `002-controller-pcb`

**Created**: 2026-10-25

**Status**: Draft

**Input**: User description: "Erfasse die aktuelle definition der Hardware nun als Anforderung um mit dem Platinendesign anzufangen."

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Platine in Betrieb nehmen (Priority: P1)

Der Benutzer bestückt die neue Controller-Platine, schließt Display, Bedienteil und die beiden
Temperaturfühler an und versorgt sie zunächst nur über USB. Er spielt die Firmware über USB auf.
Der Controller startet, zeigt Firmware-Version und erkannte PCB-Revision, reagiert auf Encoder
und Stopp-Taste und zeigt beide Ofentemperaturen an. Danach betreibt er die Platine am Netz.

**Why this priority**: Ohne funktionierende Grundplatine gibt es keine weitere Ofenfunktion. Die
Inbetriebnahme belegt, dass Versorgung, Controller, Anzeige, Bedienung und Messung auf der
eigenen Platine genauso funktionieren wie auf dem Steckbrett-Prototyp.

**Independent Test**: Platine nur über USB versorgen, Firmware aufspielen, Start beobachten,
Encoder drehen und drücken, Stopp-Taste betätigen, beide Fühler bei Raumtemperatur ablesen.
Anschließend dasselbe mit Netzversorgung. Kein Lastteil angeschlossen.

**Acceptance Scenarios**:

1. **Given** eine bestückte Platine ohne Netzanschluss, **When** sie per USB mit einem Rechner
   verbunden wird, **Then** lässt sich die Firmware ohne zusätzliche Programmierhardware
   aufspielen.
2. **Given** die Firmware ist aufgespielt, **When** die Platine startet, **Then** zeigt das
   Display Firmware-Version und PCB-Revision „REV A" an.
3. **Given** beide Fühler sind angeschlossen, **When** der Statusbereich angezeigt wird,
   **Then** erscheinen die Temperatur oben und unten, und beide weichen bei Raumtemperatur
   höchstens 3 °C von einem Referenzthermometer ab.
4. **Given** die Platine läuft, **When** der Encoder gedreht und gedrückt bzw. die Stopp-Taste
   betätigt wird, **Then** erkennt die Firmware jede Eingabe.
5. **Given** die Platine ist zusätzlich über USB verbunden, **When** die Netzversorgung
   eingeschaltet wird, **Then** läuft der Controller ohne Neustart weiter und der Rechner wird
   nicht aus dem Netzteil gespeist.

---

### User Story 2 - Hardware-Sicherheitskette (Priority: P1)

Der Benutzer verlässt sich darauf, dass die Heizung nur nach seiner ausdrücklichen Freigabe
Strom bekommt und bei Stopp, Fehler oder Übertemperatur sofort abschaltet, auch wenn die
Firmware hängt, abgestürzt ist oder gerade neu startet. Nach jedem Abschalten bleibt die Heizung
aus, bis er erneut freigibt.

**Why this priority**: Der Ofen ist ein Heizgerät mit Netzspannung (Verfassung Prinzip IVa/IVb).
Die Sicherheitskette ist Voraussetzung, bevor überhaupt eine Last angeschlossen werden darf.

**Independent Test**: Hardware-in-the-Loop-Test auf der echten Platine mit Lastteil, aber ohne
Heizstäbe (Ersatzlast, z. B. Glühlampe): Freigabe setzen, dann jede Abschaltquelle einzeln
auslösen und prüfen, dass die Last abschaltet und die Freigabe gelöscht bleibt.

**Acceptance Scenarios**:

1. **Given** die Platine wurde eingeschaltet oder neu gestartet, **When** keine Benutzeraktion
   erfolgt ist, **Then** ist die Heizungsfreigabe gelöscht, das Sicherheitsrelais offen und
   kein Heizausgang aktiv.
2. **Given** die Heizung ist freigegeben und aktiv, **When** die Stopp-Taste gedrückt wird,
   **Then** schaltet die Last ohne Mitwirkung der Firmware ab, und nach dem Loslassen bleibt sie
   aus.
3. **Given** die Heizung ist freigegeben, **When** das Kabel der Stopp-Taste abgezogen wird oder
   eine Ader bricht, **Then** wirkt das wie ein Stopp.
4. **Given** die Heizung ist freigegeben, **When** die Firmware nicht mehr regelmäßig ihr
   Lebenszeichen gibt, **Then** schaltet die Last innerhalb von 2,6 Sekunden ab.
5. **Given** die Heizung ist freigegeben, **When** die Platinentemperatur die Hardware-Schwelle
   überschreitet, **Then** schaltet die Last ohne Mitwirkung der Firmware ab und bleibt nach
   dem Abkühlen aus.
6. **Given** die Heizung ist freigegeben, **When** ein Temperaturmessbaustein eine
   Übertemperatur der Kammer oder einen Fühlerfehler meldet, **Then** schaltet die Last ohne
   Mitwirkung der Regelungslogik ab.
7. **Given** eine Abschaltquelle ist noch aktiv (Stopp gedrückt, Platine zu heiß), **When** die
   Firmware die Freigabe setzen will, **Then** bleibt die Freigabe gelöscht.
8. **Given** ein elektronischer Lastschalter ist durchlegiert (Kurzschluss), **When** die
   Freigabe gelöscht wird, **Then** trennt das Sicherheitsrelais die Heizungszuleitung.

---

### User Story 3 - Ober- und Unterhitze getrennt schalten (Priority: P2)

Der Benutzer schließt das Lastteil an. Die Firmware kann Ober- und Unterhitze unabhängig
voneinander takten, solange die Freigabe gesetzt ist.

**Why this priority**: Grundlage für jede Temperaturregelung. Setzt die Sicherheitskette aus
User Story 2 voraus.

**Independent Test**: Mit gesetzter Freigabe Ober- und Unterhitze einzeln und gemeinsam ein-
und ausschalten (Ersatzlast oder Heizstäbe) und an beiden Lastkreisen messen.

**Acceptance Scenarios**:

1. **Given** die Freigabe ist gesetzt, **When** die Firmware nur die Oberhitze einschaltet,
   **Then** führt nur der obere Lastkreis Strom.
2. **Given** die Freigabe ist gesetzt, **When** die Firmware beide Kreise im Sekundentakt
   ein- und ausschaltet, **Then** folgen beide Kreise ohne Aussetzer.
3. **Given** die Freigabe ist gelöscht, **When** die Firmware einen Heizausgang einschaltet,
   **Then** führt kein Lastkreis Strom.

---

### User Story 4 - Einbau, Wartung und Aktualisierung (Priority: P3)

Der Benutzer baut die Platine in den Bedienschacht des Ofens ein, steckt alle Kabel an und
schließt das Gehäuse. Firmware-Updates erfolgen später drahtlos; für Erstinbetriebnahme und
Wiederherstellung erreicht er den USB-Anschluss von außen. Zum Warten lässt sich die Platine
ohne Löten abklemmen und ausbauen.

**Why this priority**: Wichtig für den Dauerbetrieb, aber erst sinnvoll, wenn Platine und
Sicherheitskette funktionieren.

**Independent Test**: Platine einbauen, Gehäuse schließen, WLAN-Verbindung und
Firmware-Übertragung über USB von außen prüfen; Platine abklemmen und ausbauen.

**Acceptance Scenarios**:

1. **Given** die Platine ist im geschlossenen Gehäuse eingebaut, **When** der Ofen startet,
   **Then** verbindet er sich mit dem WLAN am üblichen Aufstellort.
2. **Given** das Gehäuse ist geschlossen, **When** ein USB-Kabel an die Buchse in der
   Gehäusewand gesteckt wird, **Then** lässt sich die Firmware aufspielen.
3. **Given** die Platine ist eingebaut, **When** sie ausgebaut werden soll, **Then** lassen
   sich alle externen Verbindungen ohne Werkzeug zum Löten trennen.

---

### Edge Cases

- **Elektronischer Lastschalter fällt kurzgeschlossen aus**: Die Firmware erkennt einen
  Temperaturanstieg ohne Heizbefehl (eigenes Feature) und löscht die Freigabe; das
  Sicherheitsrelais trennt. Versagt auch das Relais, bleibt die Thermosicherung des Ofens als
  letzte Abschaltung.
- **Ein Temperaturfühler fehlt, ist abgerissen oder unplausibel**: Meldung über den
  Messbaustein; Freigabe wird gelöscht; Fehlercode auf dem Display.
- **Ein Messbaustein fällt aus**: Für diesen Kanal fehlen Kammer- und Platinentemperatur; das
  wird wie Übertemperatur behandelt (Heizung aus).
- **Firmware startet neu (Watchdog, Absturz, Brownout, Flashen über USB)**: Heizausgänge sind
  während Reset und Start aus; die Freigabe wird beim Start gelöscht.
- **Netzspannung fällt aus oder bricht ein**: Relais fällt ab, Freigabe ist nach Wiederkehr
  gelöscht; kein automatisches Weiterheizen.
- **USB angeschlossen, kein Netz**: Controller läuft, Lastkreise sind stromlos; Relais und
  Lastschalter dürfen sich ohne Schaden mitbewegen.
- **Platine wird zu heiß, kühlt wieder ab**: Freigabe bleibt gelöscht, bis der Benutzer neu
  startet.
- **Falsche Firmware für die Platine**: Erkennt die Firmware eine nicht unterstützte
  PCB-Revision, bleibt sie im sicheren Zustand und zeigt einen Fehler (Prinzip VI).
- **Einschaltglitches der Controller-Pins**: Kurze Pegel beim Einschalten dürfen weder einen
  Heizausgang einschalten noch die Freigabe setzen.

## Requirements *(mandatory)*

### Functional Requirements

**Versorgung**

- **FR-001**: Die Platine MUSS sich aus der Netzspannung (230 V AC) über ein isoliertes,
  zertifiziertes Netzteilmodul versorgen und alle Verbraucher (Controller mit WLAN, Display,
  Messung, Sicherheitsrelais, Lastschalter-Ansteuerung, Signalgeber) einschließlich
  Stromspitzen ohne Spannungseinbruch versorgen.
- **FR-002**: Der Netzeingang der Platine MUSS durch eine eigene Sicherung und einen
  Überspannungsschutz abgesichert sein.
- **FR-003**: Die Platine MUSS ohne Netz allein über USB lauffähig sein, und die USB-Versorgung
  DARF NICHT in das Netzteil oder umgekehrt zurückspeisen.

**Controller, Programmierung und Diagnose**

- **FR-004**: Die Firmware MUSS sich über USB ohne zusätzliche Programmierhardware aufspielen
  lassen; die Platine MUSS dafür Bedienelemente für Neustart und Boot-Modus haben.
- **FR-005**: Die Platine MUSS WLAN mit einer externen Antenne unterstützen, die außerhalb des
  Metallgehäuses angebracht werden kann (Voraussetzung für drahtlose Updates).
- **FR-006**: Die Platine MUSS eine Diagnose-LED sowie eine optional bestückbare serielle
  Debug-Schnittstelle haben.
- **FR-007**: Die Platine MUSS ihre Revision so codieren, dass die Firmware sie zur Laufzeit
  erkennt (Prinzip VI); die Revision „REV A" MUSS im Bestückungsdruck stehen.
- **FR-008**: Der USB-Anschluss MUSS gegen elektrostatische Entladung geschützt sein.

**Bedienung und Anzeige**

- **FR-009**: Die Platine MUSS das vorhandene 2,8"-Display (3,3 V an allen Pins) über ein
  eigenes Kabel ansteuern, einschließlich dimmbarer und beim Start dunkler
  Hintergrundbeleuchtung.
- **FR-010**: Die Platine MUSS Drehencoder mit Taster und Stopp-Taste (Öffner und Schließer)
  über einen gemeinsamen Steckverbinder anbinden; Entprellung und ESD-Schutz der Eingänge
  liegen auf der Platine.
- **FR-011**: Der Encoder MUSS auf einem eigenen kleinen Träger in der Front montierbar sein;
  die Schritte pro Rastung MÜSSEN je PCB-Revision konfigurierbar sein.
- **FR-012**: Die Platine MUSS einen akustischen Signalgeber haben.

**Temperaturmessung**

- **FR-013**: Die Platine MUSS zwei Thermoelement-Kanäle Typ K (oben, unten) mit
  Netzfrequenz-Unterdrückung (50 Hz), Fühlerbruch- und Kurzschlusserkennung haben.
- **FR-014**: Jeder Messkanal MUSS eine einstellbare Kammer-Übertemperaturschwelle haben, deren
  Überschreiten (oder ein Fühlerfehler) die Heizungsfreigabe in Hardware löscht.
- **FR-015**: Die Platine MUSS ihre eigene Temperatur messen (Prinzip IVa); dafür DÜRFEN die
  Vergleichsstellen-Temperaturen der Messkanäle genutzt werden, sofern sie an der thermisch
  kritischen Stelle (Umgebung im Elektronikfach, Seite zur Heizkammer) liegen.
- **FR-016**: Die Fühleranschlüsse MÜSSEN steckbar sein und thermisch mit dem jeweiligen
  Messbaustein gekoppelt liegen, abseits der Wärmequellen der Platine.

**Lastansteuerung**

- **FR-017**: Die Platine MUSS zwei externe Halbleiterrelais (Ober- und Unterhitze) unabhängig
  voneinander ansteuern; jeder Heizausgang MUSS hardwareseitig standardmäßig AUS sein
  (Prinzip IVa).
- **FR-018**: Die Platine MUSS ein externes mechanisches Sicherheitsrelais in der gemeinsamen
  Heizungszuleitung ansteuern, das nur bei gesetzter Freigabe anzieht.
- **FR-019**: Ein Heizkreis DARF NUR Strom führen, wenn Freigabe UND Heizbefehl des jeweiligen
  Kanals gleichzeitig vorliegen; diese Verknüpfung MUSS in Hardware erfolgen.

**Hardware-Sicherheitskette**

- **FR-020**: Die Platine MUSS eine speichernde Heizungsfreigabe haben, die nur die Firmware
  setzen kann und die nach dem Einschalten gelöscht ist.
- **FR-021**: Folgende Quellen MÜSSEN die Freigabe ohne Mitwirkung der Firmware löschen, und
  Löschen MUSS Vorrang vor Setzen haben: Stopp-Taste, Unterbrechung der Stopp-Leitung,
  Platinen-Übertemperatur, Kammer-Übertemperatur oder Fühlerfehler (FR-014), Ausbleiben des
  Firmware-Lebenszeichens (externer Watchdog), Unterspannung der Logikversorgung.
- **FR-022**: Die Firmware MUSS die Freigabe zusätzlich selbst löschen können.
- **FR-023**: Die Firmware MUSS den Zustand der Freigabe und der Stopp-Taste zurücklesen können.
- **FR-024**: Die Platinen-Übertemperaturabschaltung MUSS ohne Firmware wirken und bei
  **65 °C** auslösen; die Firmware-Grenze für die Platinentemperatur MUSS bei **60 °C** liegen
  (beides zur Compile-Zeit bzw. ab Werk festgelegt, Prinzip IVa/VII).
- **FR-025**: Heizausgänge und Freigabe-Setzen MÜSSEN auf Controller-Pins liegen, deren
  Einschaltverhalten weder einen Heizausgang einschalten noch die Freigabe setzen kann.

**Elektrische Sicherheit und Mechanik**

- **FR-026**: Netzspannung führende Bereiche der Platine MÜSSEN normgerechte Kriech- und
  Luftstrecken zur Kleinspannungsseite einhalten (Prinzip IVb); der Laststrom der Heizung DARF
  NICHT über die Platine fließen.
- **FR-027**: Alle externen Verbindungen (Netz, Lastteil, Display, Bedienteil, Fühler, USB)
  MÜSSEN steckbar sein.
- **FR-028**: Die Platine MUSS in den Bedienschacht neben dem Garraum passen (senkrecht zur
  Front) und auf einem Montageblech befestigt werden können: Außenmaße höchstens
  150 × 150 mm, Bauteilhöhe höchstens 40 mm. Innerhalb dieser Grenzen MUSS die Fläche
  kostenoptimiert so klein wie sinnvoll gewählt werden. Platinenmaße, Bohrungen, Bauteilhöhen
  und Anschlusslage MÜSSEN mit dem Gehäuse-Feature abgestimmt sein.
- **FR-034**: Die Platine MUSS wartungsfreundlich sein: Testpunkte mindestens für alle
  Kleinspannungs-Versorgungen, die Heizungsfreigabe, die Heizausgänge und die Löschleitung.
  Im Netzspannung führenden Bereich DÜRFEN KEINE Test- oder Messpunkte vorhanden sein. Alle
  Anschlüsse am Platinenrand, nach Funktion gruppiert (Netz getrennt von Kleinspannung) und im
  Bestückungsdruck mit Funktion und Pinbelegung beschriftet.
- **FR-029**: Die Kleinspannungsmasse der Platine DARF im Betrieb NICHT mit dem Schutzleiter
  verbunden sein; die Schirmanbindung der USB-Buchse MUSS per Bestückungsoption wählbar sein.
- **FR-030**: Alle Bauteile MÜSSEN eine zulässige Betriebstemperatur von mindestens 70 °C
  haben.
- **FR-031**: Schaltplan, Layout, Footprints und Stückliste MÜSSEN als KiCad-Projekt im
  Repository liegen; die Pinbelegung MUSS als Board-Konfiguration „REV A" in der Firmware
  hinterlegt sein (Prinzip VI/VII).
- **FR-032**: Die Hardware-in-the-Loop-Tests MÜSSEN mindestens Sensorlesung beider Kanäle und
  das Abschalten über jede Quelle aus FR-021 abdecken und für REV A dokumentiert werden
  (Prinzip III).
- **FR-033**: Die Platine MUSS komplett von Hand bestückbar sein: SMD-Bauteile ausschließlich
  auf einer Platinenseite, Baugröße nicht kleiner als 0603 bzw. SOT-23 (bevorzugt), keine BGA.
  QFN-Gehäuse sind zulässig (Bestückung mit Reflow-Heizplatte), aber nur, wenn es keine
  gleichwertige Alternative in einem bedrahteten oder bedrahtungsähnlichen Gehäuse
  (SOIC, TSSOP, SOT) gibt.
- **FR-035**: Zum Feature gehört die Inbetriebnahme-Firmware für REV A: Board-Konfiguration
  REV A, Build-Umgebung für den neuen Controller, Treiber für beide Messkanäle,
  Heizungsfreigabe, externen Watchdog, Heizausgänge und Revisionserkennung sowie die HIL-Tests
  nach FR-032. Die Funktionen aus Feature 001 MÜSSEN danach auf REV A laufen.

### Key Entities

- **PCB-Revision**: Kennung der Platine (REV A); im Bestückungsdruck sichtbar und zur Laufzeit
  erkennbar; bestimmt die Board-Konfiguration der Firmware.
- **Heizungsfreigabe**: Speichernder Zustand „gesetzt/gelöscht"; nur von der Firmware setzbar,
  von mehreren Quellen löschbar; bestimmt Versorgung der Lastschalter und Sicherheitsrelais.
- **Löschquelle**: Ereignis, das die Freigabe löscht (Stopp, Leitungsbruch,
  Platinen-Übertemperatur, Kammer-Übertemperatur/Fühlerfehler, Watchdog, Unterspannung,
  Firmware).
- **Messkanal**: Thermoelement (oben bzw. unten) mit Messbaustein; liefert Kammertemperatur,
  Vergleichsstellen-Temperatur (Platinentemperatur) und Fehlerstatus.
- **Heizkreis**: Ober- bzw. Unterhitze mit eigenem Lastschalter; gemeinsame Zuleitung über das
  Sicherheitsrelais.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Nach Druck auf die Stopp-Taste ist die Last in 100 % der Versuche innerhalb von
  100 ms stromlos, auch bei angehaltener Firmware.
- **SC-002**: Nach Ausbleiben des Firmware-Lebenszeichens ist die Last innerhalb von
  2,6 Sekunden stromlos (maximale Watchdog-Zeit des Bausteins 2,5 s plus Reserve).
- **SC-003**: Jede Löschquelle aus FR-021 schaltet die Last im HIL-Test ab; in keinem Fall
  heizt der Ofen danach ohne neue Benutzerfreigabe weiter.
- **SC-004**: In 100 Einschaltvorgängen und 100 Neustarts wird kein Lastkreis auch nur kurz
  eingeschaltet.
- **SC-005**: Beide Temperaturkanäle weichen bei Raumtemperatur höchstens 3 °C von einem
  Referenzthermometer und höchstens 2 °C voneinander ab.
- **SC-006**: Ein abgezogener Fühler wird innerhalb von 2 Sekunden erkannt und löscht die
  Freigabe.
- **SC-007**: Die Firmware lässt sich bei geschlossenem Gehäuse über die USB-Buchse in der
  Gehäusewand aufspielen; der Ofen erreicht das WLAN am Aufstellort mit geschlossenem Gehäuse.
- **SC-008**: Die Platine lässt sich in weniger als 10 Minuten ohne Löten aus- und wieder
  einbauen.

## Assumptions

- Die Vorauswahl der Bauteile und ihre Begründung stehen in `docs/hardware-umbau.md` (Stand
  2026-10-25): u. a. Controller-Modul mit externer Antenne und nativem USB, zwei
  Thermoelement-Messbausteine mit 50-Hz-Filter und Fehlerausgang, Netzteilmodul 5 V / 10 W,
  LDO 3,3 V, Temperaturschalter mit fester Schwelle, externer Watchdog, Speicherbaustein für die
  Freigabe, Panel-SSRs und Sicherheitsrelais in Fassung. Die endgültige Auswahl und die
  Pinbelegung legt der Plan fest.
- Die Schwellen in FR-024 (bestätigt, siehe Clarifications) orientieren sich am
  empfindlichsten Bauteil: Das Netzteilmodul ist je nach Datenblattstand nur bis 70 °C Umgebung
  spezifiziert.
- Die Kammer-Übertemperaturschwelle der Messkanäle (FR-014), die maximale Heizdauer, die
  zulässige Abweichung zwischen oben und unten sowie die Thermal-Runaway-Erkennung legt das
  Feature „Sichere Temperaturregelung" fest. Dieses Feature stellt die Hardware dafür bereit.
- Lastteil (Kaltgerätebuchse, Sicherheitsrelais, SSRs, Verkabelung im Ofen), Montageblech und
  Gehäuse sind nicht Teil der Platine; Abmessungen und Wärmevorgaben (Prinzip X) kommen aus dem
  Feature „Controller-Gehäuse". Die Thermosicherung des Ofens bleibt in Reihe.
- Drahtlose Firmware-Updates sind ein eigenes Firmware-Feature (nur im Leerlauf bei gelöschter
  Freigabe). Dieses Feature stellt nur WLAN mit externer Antenne und USB als Rückfallweg bereit.
- Die Temperaturregelung selbst (Heizprofile, Regler, Thermal-Runaway-Logik) ist nicht Teil
  dieses Features; die Inbetriebnahme-Firmware nach FR-035 schaltet Heizausgänge nur für Tests.
- Kein Umluftlüfter (Entscheidung 2026-10-24).
- Die Platine wird bei einem Platinenhersteller gefertigt und vom Benutzer von Hand bestückt
  (siehe FR-033). Im Gehäuse ist genug Platz; Miniaturisierung ist kein Ziel.

## Clarifications

### Session 2026-10-25

- Q: Wie soll die Platine bestückt werden, von Hand oder vom Platinenhersteller? → A: Komplett
  von Hand; SMD nur auf einer Seite, mindestens 0603/SOT-23 (bevorzugt); QFN per
  Reflow-Heizplatte möglich, aber Alternativen bevorzugt.
- Q: Welche maximale Platinengröße soll gelten? → A: Höchstens 150 × 150 mm, Bauteilhöhe
  höchstens 40 mm; trotzdem kostenoptimiert nicht größer als nötig; wartungsfreundlich mit
  Testpunkten, sinnvoller Anordnung und Beschriftung der Anschlüsse. Ergänzung: keine
  Messpunkte an 230 V.
- Q: Bei welchen Platinentemperaturen soll abgeschaltet werden? → A: Firmware 60 °C,
  Hardware 65 °C (Auslegung auf 70 °C Umgebung für das Netzteilmodul).
- Q: Welche Zeit gilt für die Abschaltung bei ausbleibendem Lebenszeichen (SC-002), nachdem
  das Datenblatt des Watchdog-Bausteins geprüft ist (T006)? → A: 2,6 s statt 2 s; der Baustein
  löst spätestens nach 2,5 s aus, schnellere Varianten haben keinen Open-Drain-Ausgang.
- Q: Wie viel Firmware gehört zu diesem Platinen-Feature? → A: Hardware plus
  Inbetriebnahme-Firmware (Board-Konfiguration, Treiber, HIL-Tests); Regelung folgt im Feature
  „Sichere Temperaturregelung".
