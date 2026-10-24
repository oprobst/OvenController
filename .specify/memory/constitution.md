<!--
Sync Impact Report
==================
Version change: (Template, unversioniert) → 1.0.0
Begründung: Erstratifizierung – alle Platzhalter erstmals mit Inhalt befüllt (MAJOR-Startversion).

Geänderte Prinzipien (Template-Platzhalter → neuer Titel):
- [PRINCIPLE_1_NAME] → I. Modulare Architektur mit Schnittstellen (NICHT VERHANDELBAR)
- [PRINCIPLE_2_NAME] → II. SOLID-Prinzipien
- [PRINCIPLE_3_NAME] → III. Hardware-Abstraktion & Testbarkeit
- [PRINCIPLE_4_NAME] → IV. Thermische Sicherheit & Fail-Safe (NICHT VERHANDELBAR)
- [PRINCIPLE_5_NAME] → V. Ressourcenbewusstsein & Determinismus

Hinzugefügte Abschnitte:
- Technologie-Stack & Werkzeuge (ersetzt [SECTION_2_NAME])
- Entwicklungs-Workflow & Qualitätstore (ersetzt [SECTION_3_NAME])
- Governance (befüllt)

Entfernte Abschnitte: keine

Templates: nicht verändert (lesen die Verfassung zur Laufzeit).

Offene TODOs:
- Konkreter Microcontroller ist bewusst noch nicht festgelegt; Auswahl erfolgt im ersten
  Feature-Plan und muss Abschnitt „Technologie-Stack & Werkzeuge" genügen.
-->

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

**Begründung**: Host-Tests sind schnell, reproduzierbar und erlauben das Prüfen von
Fehlerfällen, die auf echter Hardware gefährlich oder schwer erzeugbar sind.

### IV. Thermische Sicherheit & Fail-Safe (NICHT VERHANDELBAR)

- Der sichere Zustand ist „Heizung AUS". Bei jedem erkannten Fehler (Sensorfehler,
  Messwert außerhalb des Plausibilitätsbereichs, Übertemperatur, Kommunikationsfehler,
  Watchdog-Reset) MUSS die Firmware in diesen Zustand wechseln und den Fehler anzeigen.
- Eine harte Maximaltemperatur MUSS unabhängig von der Regelungslogik überwacht werden.
- Ein Hardware-Watchdog MUSS aktiv sein; nach einem Reset startet die Heizung NICHT
  automatisch wieder.
- Eine Thermal-Runaway-Erkennung (Heizung aktiv, aber kein erwarteter Temperaturanstieg)
  MUSS vorhanden sein.
- Das PCB-Design MUSS eine hardwareseitige Abschaltmöglichkeit vorsehen (z. B.
  Thermosicherung/Übertemperaturschalter), die ohne Firmware wirkt; Netzspannung führende
  Bereiche MÜSSEN normgerechte Kriech- und Luftstrecken einhalten.

**Begründung**: Ein Ofen ist ein Heizgerät mit Brand- und Verletzungsrisiko; Sicherheit hat
Vorrang vor jeder Funktionalität.

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

## Entwicklungs-Workflow & Qualitätstore

- Jede Änderung durchläuft den Spec-Kit-Ablauf: Spezifikation → Plan → Tasks → Umsetzung.
- Vor jedem Merge MÜSSEN erfüllt sein:
  - Firmware baut fehlerfrei für alle Zielumgebungen.
  - Alle `native`-Tests sind grün.
  - Keine neuen Warnungen von Compiler (`-Wall -Wextra`) und statischer Analyse.
  - Neue/geänderte Schnittstellen sind dokumentiert (Vertrag, Einheiten, Fehlerverhalten).
- Änderungen an Schnittstellen zwischen Firmware und Hardware (Pinbelegung, Sensor-,
  Aktor- oder Spannungswahl) MÜSSEN in Firmware, KiCad-Projekt und Dokumentation
  konsistent nachgezogen werden.
- Mechanische Abhängigkeiten zwischen PCB und Gehäuse (Platinenmaße, Bohrungen, Bauteilhöhen,
  Anschlüsse) MÜSSEN zwischen KiCad und Fusion 360 abgeglichen werden.
- Sicherheitsrelevante Änderungen (Prinzip IV) erfordern eine explizite Review-Prüfung
  gegen die Fail-Safe-Anforderungen.

## Governance

- Diese Verfassung hat Vorrang vor allen anderen Praktiken und Richtlinien des Projekts.
- Änderungen erfolgen über `/speckit-constitution`, werden mit Begründung dokumentiert und
  benötigen die Zustimmung des Projektverantwortlichen.
- Versionierung nach Semantic Versioning:
  - MAJOR: Entfernen oder grundlegende Neudefinition von Prinzipien.
  - MINOR: Neue Prinzipien/Abschnitte oder wesentlich erweiterte Vorgaben.
  - PATCH: Klarstellungen, Formulierungen, Tippfehler.
- Jeder Plan (`/speckit-plan`) MUSS einen Constitution-Check enthalten; Abweichungen sind nur
  mit dokumentierter Begründung im Abschnitt „Complexity Tracking" zulässig.
- Reviews MÜSSEN die Einhaltung der Prinzipien I–V prüfen.

**Version**: 1.0.0 | **Ratified**: 2026-10-24 | **Last Amended**: 2026-10-24
