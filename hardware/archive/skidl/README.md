# Archiv: Schaltplan als SKiDL-Code (Feature 002, T010–T018)

Bis 2026-10-25 war dieser Python-Code die Quelle des Schaltplans. Er ist geprüft (SKiDL-ERC,
GPIO-Abgleich mit der Firmware, Footprint-Prüfung mit KiCad 10) und hat die Referenznetzlisten
`hardware/controller/controller.ref.net` und `hardware/encoder-carrier/encoder-carrier.ref.net`
erzeugt. Daraus wurde einmalig die Vorlage des KiCad-Schaltplans generiert.

**Seitdem ist der KiCad-Schaltplan die Quelle** (siehe `docs/werkzeuge-pcb.md`). Dieser Code wird
nicht mehr geändert und nicht mehr ausgeführt. Er bleibt zum Nachvollziehen der Entscheidungen
(Kommentare mit Verweisen auf `specs/002-controller-pcb/research.md`).

Ausführen nur noch zu Archivzwecken: `requirements.txt` (SKiDL 3.0.0) in einer eigenen
Python-Umgebung installieren; die Skripte erwarten ihre ursprünglichen Pfade unter
`hardware/<platine>/schematic/`.
