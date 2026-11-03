# carinthiamesh-sensor-nodes

Beispiele für Sensor- und Aktor-Nodes im [MeshCore](https://github.com/meshcore-dev/MeshCore)-Netz, gesammelt von der CarinthiaMesh-Initiative in Kärnten. Jedes Beispiel ist ein kleiner, nachbaubarer Aufbau: Hardware, Schaltplan, Code und eine Schritt-für-Schritt-Anleitung.

*English: A collection of MeshCore sensor and actuator node examples from the CarinthiaMesh initiative (Carinthia, Austria). Each example ships wiring, a schematic, firmware changes and a step-by-step guide. Documentation is in German.*

## Beispiele

| Beispiel | Hardware | Was es macht | Status |
| --- | --- | --- | --- |
| [button-sensor](examples/button-sensor/) | Heltec V3, 4 Taster, LED, Buzzer | Tastendruck als DM-Alert, LED und Buzzer per Admin-Befehl schaltbar | kompiliert, nicht auf Hardware getestet |
| [door-sensor](examples/door-sensor/) | Heltec V3, Reed-Kontakt | Tür oder Fenster auf/zu als DM-Alert | kompiliert, nicht auf Hardware getestet |
| [leak-sensor](examples/leak-sensor/) | Heltec V3, Wassersensor-Modul | Wasser erkannt als High-Priority-Alert bis zum ACK | kompiliert, nicht auf Hardware getestet |
| [climate-sensor](examples/climate-sensor/) | Heltec V3, BME280 | Temperatur, Feuchte, Druck per Telemetrie, Alerts bei Frost und hoher Feuchte | kompiliert, nicht auf Hardware getestet |

Jedes Beispiel hat eine eigene Stückliste (`BOM.md`). Verwandte Projekte aus der Community stehen in [COMMUNITY.md](COMMUNITY.md).

Weitere Beispiele sind willkommen, siehe [Mitmachen](#mitmachen).

## Wie Sensor-Nodes bei MeshCore funktionieren

Alle Beispiele bauen auf der offiziellen Vorlage [`examples/simple_sensor`](https://github.com/meshcore-dev/MeshCore/tree/main/examples/simple_sensor) auf. Eine Einführung gibt der [MeshCore-Blog: Sensor Node Intro](https://blog.meshcore.io/2026/09/14/sensor-intro).

- **Eigener Build pro Anwendung.** Für Sensor-Nodes gibt es keine fertige Firmware im Flasher. Man passt `main.cpp` an und baut mit PlatformIO.
- **Alerts gehen als Direktnachricht** an alle Companions, die sich mit dem Admin-Passwort am Sensor angemeldet haben. Niedrige Priorität sendet einmal, hohe wiederholt bis zum ACK.
- **Telemetrie** (Akku, Umweltsensoren) können angemeldete Companions abfragen.
- **Eigene Befehle** in `handleCustomCommand()` machen aus dem Sensor einen Aktor, gesteuert über die Admin-Konsole der App.

### Warum nicht mit der Standard-Firmware?

Keine der fertigen MeshCore-Firmwares (Companion, Repeater, Room Server, Sensor) fragt externe GPIOs ab. Ein Taster oder Kontakt an einem freien Pin wird dort einfach ignoriert. Dafür gibt es den offenen Feature-Request [GPIO support in MeshCore (#3522)](https://github.com/meshcore-dev/MeshCore/issues/3522), GPIOs künftig per CLI zu lesen und zu schalten. Bis das umgesetzt ist, braucht es eigenen Code wie in diesen Beispielen.

## Spielregeln für das Mesh

Solange Sensor und Empfänger nebeneinander liegen, ist alles Zero-Hop und das Netz merkt nichts. Sobald ein Node über Repeater läuft, kostet jede Nachricht Airtime auf allen Repeatern im Pfad. Ohne bekannten Pfad geht sie sogar per Flood durchs ganze Netz.

- Direktnachricht statt Public-Kanal: keine Sensor- oder Testdaten in öffentliche Kanäle
- Rate-Limit ist Pflicht: Entprellen, Cooldown und ein Stundenbudget
- Nur Änderungen melden, Messwerte selten und gesammelt
- Wenige Empfänger, denn jeder bekommt jede Meldung einzeln
- Neue Nodes kurz in der lokalen Community ankündigen

## Mitmachen

Ein neues Beispiel bekommt einen eigenen Ordner unter `examples/<name>/` mit:

| Datei | Inhalt |
| --- | --- |
| `BOM.md` | Stückliste |
| `README.md` | Steckbrief: Hardware, Build-Target, was gemeldet wird, Airtime-Schutz, getesteter MeshCore-Stand, Status |
| `anleitung.md` | Verdrahtung, Aufbau, Build, Test |
| `schaltplan.svg` | Schaltplan |
| `firmware/` | geänderte `main.cpp` und ein Patch gegen `simple_sensor` |

Jedes Beispiel muss die Spielregeln oben einhalten. Pull Requests und Issues sind willkommen, ebenso Rückmeldungen, ob ein Beispiel auf echter Hardware läuft.

## Lizenz

MIT, siehe [LICENSE](LICENSE). Die Firmware-Dateien basieren auf MeshCore (MIT, Scott Powell / rippleradios.com).
