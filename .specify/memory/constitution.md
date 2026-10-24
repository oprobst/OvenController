# Mini-Ofen-Controller Constitution

## Core Principles

### I. Modulare Architektur mit Schnittstellen (NICHT VERHANDELBAR)

- Die Firmware MUSS in klar abgegrenzte Module gegliedert sein (z. B. Temperaturmessung,
  Heizungsansteuerung, Regelung, Benutzeroberfläche, Ablaufsteuerung, Persistenz).
- Jedes Modul MUSS ausschließlich über eine öffentliche Schnittstelle (abstrakte C++-Klasse
  bzw. reiner Header mit Interface-Definition) ansprechbar sein. Implementierungsdetails
  liegen in privaten Headern/Quellen und DÜRFEN NICHT von anderen Modulen inkludiert werden.
- Module MÜSSEN als eigenständige PlatformIO-Bibliotheken unter `lib/` organisiert sein,
  mit getrennten Verzeichnissen für Schnittstelle (`include/`) und Implementierung (`src/`).
- Zirkuläre Abhängigkeiten zwischen Modulen sind verboten.
- Globale veränderliche Zustände außerhalb des Composition Root (`main`) sind verboten.

**Begründung**: Klare Modulgrenzen ermöglichen isolierte Tests, Austausch von Hardware und
paralleles Arbeiten, ohne unbeabsichtigte Kopplung.

### II. SOLID-Prinzipien

- **Single Responsibility**: Jede Klasse/jedes Modul hat genau einen Änderungsgrund.
- **Open/Closed**: Erweiterungen (z. B. neuer Sensortyp, neues Heizprofil) MÜSSEN durch neue
  Implementierungen einer Schnittstelle erfolgen, nicht durch Änderung bestehender Logik.
- **Liskov Substitution**: Jede Implementierung MUSS den dokumentierten Vertrag ihrer
  Schnittstelle vollständig erfüllen (inkl. Fehlerverhalten und Einheiten).
- **Interface Segregation**: Schnittstellen MÜSSEN schmal und rollenspezifisch sein; kein
  Modul darf von Methoden abhängen, die es nicht nutzt.
- **Dependency Inversion**: Höhere Logik hängt nur von Abstraktionen ab. Abhängigkeiten
  werden per Konstruktor-Injektion übergeben; die Verdrahtung erfolgt zentral im
  Composition Root.

**Begründung**: SOLID ist explizite Projektvorgabe und Voraussetzung für Prinzip I und III.

### III. Hardware-Abstraktion & Testbarkeit

- Jeder Hardwarezugriff (GPIO, ADC, SPI/I²C, Timer, PWM, Display, Taster) MUSS hinter einer
  Hardware-Abstraktionsschnittstelle (HAL) liegen. Anwendungslogik DARF NICHT direkt auf
  Register oder Framework-APIs (z. B. Arduino) zugreifen.
- Anwendungs- und Regelungslogik MUSS ohne Zielhardware auf dem Host testbar sein
  (PlatformIO-Umgebung `native` mit Unity oder GoogleTest).
- Jedes Modul MUSS Unit-Tests gegen seine Schnittstelle besitzen; Hardware wird durch
  Mocks/Fakes ersetzt.
- Sicherheitsrelevante Logik (Prinzip IV) MUSS testgetrieben entwickelt werden:
  Test zuerst, Test schlägt fehl, dann Implementierung.
- Ergänzend MUSS es Hardware-in-the-Loop-Tests (HIL) auf der echten Platine geben
  (PlatformIO-Testumgebung für das Zielboard). Sie MÜSSEN mindestens die Sensorlesung und
  das sichere Abschalten der Heizung (inkl. Auslösen durch einen simulierten Fehler) prüfen.
- HIL-Tests MÜSSEN vor jedem Firmware-Release und nach jeder neuen PCB-Revision
  durchgeführt und das Ergebnis dokumentiert werden.

**Begründung**: Host-Tests sind schnell, reproduzierbar und erlauben das Prüfen von
Fehlerfällen, die auf echter Hardware gefährlich oder schwer erzeugbar sind; HIL-Tests
belegen, dass HAL und reale Hardware tatsächlich zusammenspielen.

### IVa. Thermische Sicherheit & Fail-Safe (NICHT VERHANDELBAR)

Verweise auf „Prinzip IV" in diesem Dokument umfassen IVa und IVb gemeinsam.

- Der sichere Zustand ist „Heizung AUS". Bei jedem erkannten Fehler (Sensorfehler,
  Messwert außerhalb des Plausibilitätsbereichs, Übertemperatur, Kommunikationsfehler,
  Watchdog-Reset) MUSS die Firmware in diesen Zustand wechseln und den Fehler anzeigen.
- Der Heizungsausgang MUSS hardwareseitig standardmäßig AUS sein (z. B. Pull-down an der
  SSR-/Relais-Ansteuerung): Solange der Microcontroller im Reset ist, bootet, geflasht wird,
  abgestürzt ist oder seine Pins hochohmig sind, DARF die Heizung NICHT einschalten.
- Eine harte Maximaltemperatur MUSS unabhängig von der Regelungslogik überwacht werden.
- Ein Hardware-Watchdog MUSS aktiv sein.
- Die Brownout-Erkennung des Microcontrollers MUSS aktiviert sein.
- Nach jedem Neustart (Watchdog-Reset, Brownout, Stromausfall, Einschalten) MUSS die
  Firmware im Zustand „Heizung AUS" starten; ein laufender Heizvorgang DARF NICHT
  automatisch fortgesetzt werden, sondern erfordert eine erneute Benutzeraktion.
- Eine Thermal-Runaway-Erkennung (Heizung aktiv, aber kein erwarteter Temperaturanstieg)
  MUSS vorhanden sein.
- Jeder Heizvorgang MUSS nach einer maximalen Betriebsdauer automatisch beendet werden.
  Die Höchstdauer MUSS zur Compile-Zeit festgelegt sein (Prinzip VII) und DARF NICHT durch
  Heizprofile oder Benutzereingaben überschritten werden; der konkrete Wert wird in der
  Spezifikation festgelegt.
- Das PCB-Design MUSS eine hardwareseitige Abschaltmöglichkeit vorsehen (z. B.
  Thermosicherung/Übertemperaturschalter), die ohne Firmware wirkt.
- Die Platine MUSS ihre eigene Temperatur mit mindestens einem Sensor auf dem PCB messen,
  platziert an der thermisch kritischsten Stelle (z. B. nahe SSR/Relais, Netzteil oder der
  der Heizkammer zugewandten Seite).
- Überschreitet die PCB-Temperatur eine Grenztemperatur, MUSS die Heizung abgeschaltet und
  in den verriegelnden Fehlerzustand (Prinzip VIII) mit eigenem Fehlercode (Prinzip IX)
  gewechselt werden. Diese Überwachung MUSS unabhängig von der Regelungslogik laufen.
- Die PCB-Grenztemperatur MUSS zur Compile-Zeit festgelegt sein (Prinzip VII) und unterhalb
  der zulässigen Betriebstemperatur der empfindlichsten Bauteile liegen; der konkrete Wert
  wird in der Spezifikation festgelegt.
- Ein Ausfall oder unplausibler Messwert des PCB-Temperatursensors MUSS wie eine
  Übertemperatur behandelt werden.
- Die PCB-Übertemperaturabschaltung SOLLTE zusätzlich hardwareseitig wirken (z. B.
  Thermostat-IC oder Komparator, der die Heizungsfreigabe ohne Firmware sperrt);
  ein Verzicht ist im Plan zu begründen.

**Begründung**: Ein Ofen ist ein Heizgerät mit Brand- und Verletzungsrisiko; Sicherheit hat
Vorrang vor jeder Funktionalität. Hardwareseitige Defaults, Neustartverhalten und
Höchstlaufzeit decken Situationen ab, in denen die Firmware nicht oder nicht korrekt läuft
oder der Ofen unbeaufsichtigt ist. Die Überwachung der PCB-Temperatur erkennt Überhitzung
der Elektronik selbst (z. B. durch SSR-Verlustleistung oder Wärme aus der Heizkammer).

### IVb. Elektrische Sicherheit (NICHT VERHANDELBAR)

- Netzspannung führende Bereiche MÜSSEN normgerechte Kriech- und Luftstrecken einhalten.
- Netzspannungsseite und Logik (Microcontroller, Sensorik, Bedienelemente) MÜSSEN galvanisch
  getrennt sein (z. B. isoliertes Netzteil, Optokoppler bzw. SSR mit integrierter Isolation).
- Der Netzpfad MUSS durch eine passend dimensionierte Sicherung abgesichert sein.
- Ein leitfähiges (Metall-)Gehäuse MUSS mit dem Schutzleiter verbunden sein.
- Die Heizlast MUSS über ein Solid-State-Relais oder ein Relais geschaltet werden, dessen
  Nennstrom deutlich über dem maximalen Laststrom liegt; die Reserve und ggf. nötige Kühlung
  (SSR-Kühlkörper) MÜSSEN im Plan begründet werden.
- Firmware DARF KEINE Annahmen über elektrische Sicherheitsmaßnahmen treffen, die nicht im
  KiCad-Projekt dokumentiert sind.

**Begründung**: Der Ofen wird mit Netzspannung betrieben; Stromschlag- und Brandrisiko
müssen durch die Hardware selbst beherrscht werden, unabhängig von der Firmware.

### V. Ressourcenbewusstsein & Determinismus

- Nach der Initialisierung DARF KEINE dynamische Speicherallokation (`new`, `malloc`) erfolgen.
- Exceptions und RTTI MÜSSEN deaktiviert sein; Fehler werden über Rückgabewerte/Statustypen
  gemeldet.
- Die Hauptschleife und Regelungszyklen MÜSSEN nicht-blockierend sein (kein `delay()` in
  Anwendungslogik); zeitkritische Aufgaben laufen mit definierter Periode.
- Physikalische Größen MÜSSEN mit eindeutigen Einheiten typisiert oder benannt sein
  (z. B. `temperatureCelsius`, `periodMs`).

**Begründung**: Vorhersagbares Zeit- und Speicherverhalten ist auf Microcontrollern
Grundlage für zuverlässige Regelung und Sicherheit.

### VI. Revisionskonsistenz über Firmware, PCB und Gehäuse

- Jede PCB-Revision MUSS eine eindeutige Kennung tragen, die auf der Platine sichtbar ist
  (z. B. im Silkscreen, Format `REV A`, `REV B`, …).
- Die Firmware MUSS die PCB-Revision kennen: entweder per Hardware-Erkennung zur Laufzeit
  (z. B. Codier-Widerstände/Strapping-Pins) oder fest zur Compile-Zeit (eigene
  PlatformIO-Umgebung bzw. Build-Flag je Revision).
- Erkennt die Firmware zur Laufzeit eine nicht unterstützte PCB-Revision, MUSS sie im
  sicheren Zustand (Prinzip IV) bleiben und den Fehler anzeigen.
- Revisionsabhängige Unterschiede (Pinbelegung, Bauteilwahl) MÜSSEN ausschließlich in der
  HAL bzw. revisionsspezifischer Konfiguration gekapselt sein, nicht in der Anwendungslogik.
- Firmware-Versionen folgen Semantic Versioning; Gehäuse-Revisionen tragen ebenfalls eine
  eindeutige Kennung.
- Beim Start MUSS die Firmware ihre Version und die erkannte bzw. konfigurierte
  PCB-Revision auf der Benutzeroberfläche anzeigen und (sofern aktiv) ins Log schreiben.
- Eine Kompatibilitätsmatrix (`docs/compatibility.md`) MUSS festhalten, welche
  Firmware-Versionen zu welchen PCB- und Gehäuse-Revisionen passen, und bei jeder neuen
  Revision bzw. jedem Firmware-Release aktualisiert werden.

**Begründung**: Hardware, Firmware und Gehäuse entwickeln sich getrennt weiter; nur eine
eindeutige Zuordnung verhindert, dass falsche Firmware auf eine Platine gelangt oder eine
Platine nicht ins Gehäuse passt.

### VII. Zentrale Konfiguration & geschützte persistente Daten

- Magic Numbers im Anwendungscode sind verboten. Grenzwerte, Reglerparameter, Zeitkonstanten
  und Pinbelegung MÜSSEN als benannte, typisierte Konstanten (`constexpr`) an einer zentralen
  Stelle definiert sein.
- Pinbelegung und hardwareabhängige Parameter MÜSSEN je Board-Revision in genau einer
  Konfigurationsdatei liegen (z. B. ein Header pro PCB-Revision, vgl. Prinzip VI).
- Sicherheitsgrenzen (z. B. harte Maximaltemperatur aus Prinzip IV) MÜSSEN zur Compile-Zeit
  festgelegt sein und DÜRFEN NICHT durch gespeicherte oder vom Benutzer eingegebene Werte
  überschrieben werden.
- Persistent gespeicherte Daten (z. B. Heizprofile, Kalibrierung, Einstellungen) MÜSSEN eine
  Schema-Version und eine Prüfsumme (z. B. CRC) besitzen.
- Beim Laden MÜSSEN Prüfsumme, Schema-Version und Wertebereiche validiert werden. Sind die
  Daten beschädigt, unplausibel oder in einer unbekannten Schema-Version, MUSS die Firmware
  auf sichere, zur Compile-Zeit definierte Standardwerte zurückfallen und dies anzeigen.
- Ändert sich das Schema, MUSS entweder eine Migration der alten Version erfolgen oder
  definiert auf Standardwerte zurückgesetzt werden; das Verhalten ist im Plan festzulegen.
- Der Zugriff auf persistente Daten erfolgt ausschließlich über ein eigenes Modul mit
  Schnittstelle (Prinzip I); Lade-, Validierungs- und Fallback-Logik MUSS per Host-Test
  abgedeckt sein (Prinzip III).

**Begründung**: Zentrale Konfiguration macht Parameter auffindbar und revisionssicher;
geschützte persistente Daten verhindern, dass ein beschädigter Speicher zu gefährlichem
Heizverhalten führt.

### VIII. Explizite Zustandsmaschine für die Ablaufsteuerung

- Die Betriebszustände des Ofens (z. B. Idle, Aufheizen, Halten, Abkühlen, Fehler; die
  endgültige Liste legt die Spezifikation fest) MÜSSEN als explizite Zustandsmaschine
  umgesetzt sein: aufgezählte Zustände (`enum class`), definierte Ereignisse und eine
  zentrale, nachvollziehbare Übergangslogik (z. B. Übergangstabelle).
- Implizite Zustände über verstreute Flags oder verschachtelte Bedingungen sind verboten.
- Nicht definierte Übergänge MÜSSEN abgewiesen werden und DÜRFEN den Zustand NICHT ändern.
- Die Heizung DARF nur in Zuständen freigegeben sein, die dies ausdrücklich vorsehen; jeder
  andere Zustand erzwingt „Heizung AUS" (Prinzip IV).
- Aus jedem Zustand MUSS ein Übergang in den Fehlerzustand möglich sein. Der Fehlerzustand
  ist verriegelnd: Er wird nur durch eine explizite Benutzerquittierung verlassen, und nur
  wenn die Fehlerursache nicht mehr besteht.
- Die Zustandsmaschine MUSS in `docs/state-machine.md` mit Diagramm (z. B. Mermaid) und
  Übergangstabelle dokumentiert sein; Code und Dokumentation MÜSSEN übereinstimmen.
- Host-Tests (Prinzip III) MÜSSEN jeden definierten Übergang abdecken, insbesondere den
  Übergang in den Fehlerzustand aus jedem einzelnen Zustand sowie das Abweisen nicht
  erlaubter Übergänge.

**Begründung**: Eine explizite Zustandsmaschine macht das Verhalten des Ofens vollständig
überprüfbar und stellt sicher, dass kein Zustand existiert, aus dem ein Fehler nicht zur
sicheren Abschaltung führt.

### IX. Diagnose & einheitliche Fehlercodes

- Die Firmware MUSS eine serielle Debug-Schnittstelle mit strukturiertem Logging bieten.
  Jede Log-Zeile MUSS mindestens Zeitstempel (ms seit Start), Log-Level, Modul und Meldung
  enthalten, in einem einheitlichen, maschinenlesbaren Format (z. B. `key=value`).
- Logging erfolgt ausschließlich über eine Logger-Schnittstelle (Prinzip I); Module DÜRFEN
  NICHT direkt auf die serielle Schnittstelle schreiben.
- Log-Level MÜSSEN zur Compile-Zeit konfigurierbar sein; im Release-Build MUSS das Logging
  per Build-Flag vollständig abschaltbar sein, ohne dass sich das funktionale Verhalten ändert.
- Logging DARF NICHT blockieren und DARF das Zeitverhalten von Regelung und
  Sicherheitsüberwachung nicht beeinträchtigen (Prinzip V).
- Fehlercodes MÜSSEN zentral an genau einer Stelle definiert sein (`enum class`) und in
  `docs/error-codes.md` mit Bedeutung, Ursache und Abhilfe dokumentiert werden.
- Fehlercodes sind stabil: Ein vergebener Code DARF NICHT umgedeutet oder wiederverwendet
  werden.
- Jeder Fehler, der zum Fehlerzustand führt (Prinzip VIII), MUSS mit seinem Code auf der
  Benutzeroberfläche angezeigt werden; diese Anzeige MUSS auch im Release-Build ohne
  Logging erhalten bleiben.

**Begründung**: Einheitliche Diagnose und stabile Fehlercodes machen Fehler im Feld und auf
dem Labortisch schnell auffindbar, ohne den Release-Build zu belasten.

### X. Thermisches Gehäusedesign

- Die Elektronik (PCB, Netzteil, Bedienelemente) MUSS thermisch von der Heizkammer getrennt
  sein (räumlicher Abstand, Isolationsschicht bzw. Hitzeschild und/oder getrennte
  Luftführung).
- Werkstoffe in Ofennähe MÜSSEN für die dort auftretende Maximaltemperatur mit Reserve
  geeignet sein; PLA und andere Kunststoffe mit niedriger Erweichungstemperatur sind dort
  verboten. Kunststoffteile im Elektronikbereich MÜSSEN ebenfalls für die dort erwartete
  Temperatur ausgelegt sein.
- Jede Gehäuse-Spezifikation MUSS verbindliche Vorgaben enthalten für: maximale
  Umgebungstemperatur der Elektronik, Lüftung (passiv/aktiv, Luftweg, Öffnungen) sowie
  maximale Bauteilhöhe und Mindestabstände zwischen PCB, Gehäusewand und Heizkammer.
- Diese Vorgaben MÜSSEN im KiCad-Projekt (Keepout-/Höhenvorgaben) und im Fusion-360-Modell
  konsistent umgesetzt und gegeneinander geprüft werden.
- Die Einhaltung der zulässigen Elektroniktemperatur MUSS durch eine Messung im Dauerbetrieb
  bei maximaler Ofentemperatur nachgewiesen werden, bevor eine Gehäuse-Revision als
  freigegeben gilt.

**Begründung**: Hitze aus der Kammer gefährdet Elektronik und Gehäuse; ungeeignete
Werkstoffe verformen sich oder werden zur Brandquelle.

## Technologie-Stack & Werkzeuge

- **Firmware**: C++ (mindestens C++17, soweit vom Toolchain unterstützt); C nur für
  herstellernahe Low-Level-Teile innerhalb der HAL.
- **Build-System**: PlatformIO. Es MÜSSEN mindestens eine Zielumgebung (Microcontroller)
  und eine `native`-Testumgebung in `platformio.ini` definiert sein.
- **Microcontroller**: Auswahl erfolgt im ersten Feature-Plan; der Wechsel des MCU DARF nur
  die HAL-Implementierungen betreffen.
- **Elektronik/PCB**: KiCad. Schaltplan, Layout, Footprints und BOM liegen versioniert im
  Repository.
- **Gehäuse/CAD**: Fusion 360. Konstruktionsdaten und exportierte Fertigungsdateien
  (z. B. STEP, STL) liegen versioniert im Repository.
- **Repository-Struktur**: `firmware/` (PlatformIO-Projekt), `hardware/` (KiCad),
  `mechanical/` (Fusion 360), `docs/` (Dokumentation).
- **Code-Stil**: Ein einheitliches `clang-format` und statische Analyse (z. B. `clang-tidy`
  oder `cppcheck`) MÜSSEN konfiguriert sein.
- **Coding-Konventionen**:
  - Sprache im Code (Bezeichner, Kommentare, Log-Meldungen) ist Englisch.
  - Sprache der Projektdokumentation (`docs/`, Spezifikationen, Pläne, diese Verfassung)
    ist Deutsch.
  - Interfaces tragen das Präfix `I` (z. B. `ITemperatureSensor`, `IHeater`).
  - Öffentliche Schnittstellen MÜSSEN mit Doxygen-Kommentaren dokumentiert sein (Zweck,
    Parameter mit Einheiten, Rückgabewerte, Fehlerverhalten).
- **Versionskontrolle & CI**: Das Projekt MUSS in einem Git-Repository verwaltet werden. Bei
  jedem Push MUSS eine CI-Pipeline (z. B. GitHub Actions) automatisch Build aller
  Zielumgebungen, `native`-Tests und statische Analyse ausführen.

## Entwicklungs-Workflow & Qualitätstore

- Jede Änderung durchläuft den Spec-Kit-Ablauf: Spezifikation → Plan → Tasks → Umsetzung.
- Vor jedem Merge MÜSSEN erfüllt sein:
  - Die CI-Pipeline ist grün.
  - Firmware baut fehlerfrei für alle Zielumgebungen.
  - Alle `native`-Tests sind grün.
  - Keine neuen Warnungen von Compiler (`-Wall -Wextra`) und statischer Analyse.
  - Neue/geänderte Schnittstellen sind dokumentiert (Vertrag, Einheiten, Fehlerverhalten).
- Änderungen an Schnittstellen zwischen Firmware und Hardware (Pinbelegung, Sensor-,
  Aktor- oder Spannungswahl) MÜSSEN in Firmware, KiCad-Projekt und Dokumentation
  konsistent nachgezogen werden.
- Mechanische Abhängigkeiten zwischen PCB und Gehäuse (Platinenmaße, Bohrungen, Bauteilhöhen,
  Anschlüsse) MÜSSEN zwischen KiCad und Fusion 360 abgeglichen werden.
- Sicherheitsrelevante Änderungen (Prinzip IV) erfordern eine explizite Prüfung gegen die
  Anforderungen aus IVa und IVb anhand einer Checkliste (z. B. per `/speckit-checklist`).
- Vor jedem Firmware-Release MÜSSEN zusätzlich die HIL-Tests (Prinzip III) bestanden sein.

## Governance

- Diese Verfassung hat Vorrang vor allen anderen Praktiken und Richtlinien des Projekts.
- Änderungen erfolgen über `/speckit-constitution`, werden mit Begründung dokumentiert und
  benötigen die Zustimmung des Projektverantwortlichen.
- Versionierung nach Semantic Versioning:
  - MAJOR: Entfernen oder grundlegende Neudefinition von Prinzipien.
  - MINOR: Neue Prinzipien/Abschnitte oder wesentlich erweiterte Vorgaben.
  - PATCH: Klarstellungen, Formulierungen, Tippfehler.
- Jeder Plan (`/speckit-plan`) MUSS einen Constitution-Check enthalten. Abweichungen sind
  nur mit dokumentierter Begründung im Abschnitt „Complexity Tracking" zulässig; für die als
  NICHT VERHANDELBAR markierten Prinzipien (I, IVa, IVb) sind Abweichungen ausgeschlossen.
  Regeln mit SOLLTE dürfen mit Begründung im Plan entfallen.
- Das Projekt wird von einer Person entwickelt. Ein Review besteht daher aus einer
  Selbstprüfung gegen die Prinzipien I–X sowie einer werkzeuggestützten Prüfung
  (z. B. `/speckit-analyze` für Spezifikation/Plan/Tasks und `/code-review` für Code);
  gefundene Abweichungen MÜSSEN vor dem Merge behoben oder begründet werden.

**Version**: 1.8.0 | **Ratified**: 2026-10-24 | **Last Amended**: 2026-10-24
