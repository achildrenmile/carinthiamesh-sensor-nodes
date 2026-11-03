# Community-Projekte und Credits

Weiterführende MeshCore-Sensor-Projekte aus der Community. Wir verlinken sie nur und kopieren keinen Code. Die Beispiele in diesem Repo sind eigene Aufbauten auf Basis der offiziellen Vorlage.

| Projekt | Von | Was | Lizenz |
| --- | --- | --- | --- |
| [MeshCore `simple_sensor`](https://github.com/meshcore-dev/MeshCore/tree/main/examples/simple_sensor) | Scott Powell und Liam Cottle | offizielle Vorlage für Sensor-Nodes, Basis aller Beispiele hier | MIT |
| [Sensor Node Intro](https://blog.meshcore.io/2026/09/14/sensor-intro) | Scott Powell, MeshCore-Blog | Einführung: Alerts, Telemetrie, Zeitreihen, eigene Befehle, Liste der unterstützten Sensoren | siehe Blog |
| [meshcore-sensor-net](https://github.com/bertrik/meshcore-sensor-net) | bertrik | Proof of Concept: STM32WLE5-Sensor sendet MeshCore-kompatible Pakete, Gateway leitet per MQTT an eine Bridge für Citizen Science. Doku: [revspace.nl/Meshcore-sensor-net](https://revspace.nl/Meshcore-sensor-net) | MIT |
| [Sensor Data Over MeshCore](https://gaggl.com/blogs/2026-08-23-sensor-telemetry-over-meshcore/) | Leo Gaggl | Blog-Artikel zu Pull-basierter Telemetrie und CayenneLPP, mit Beispiel-Payload | CC BY-NC-SA 4.0 (Website) |
| [RoadNode](https://github.com/mcfitz2/RoadNode) | mcfitz2 | eigene Sensor-Firmware, die MeshCore als unveränderte Submodule einbindet (Issue #30) | im Repo prüfen |
| [edgecollective/MeshCore-simple-sensor](https://github.com/edgecollective/MeshCore-simple-sensor) | edgecollective | Fork von MeshCore mit dem Sensor-Beispiel | MIT (wie MeshCore) |

## Hinweise

- Die Einträge stammen aus einer Websuche am 3. November 2026. Nicht jedes Projekt wurde im Detail geprüft, besonders RoadNode und der Fork sind nur an der Beschreibung erkannt.
- Eine fertige Stückliste lieferte keines dieser Projekte. Die Stücklisten in `examples/*/BOM.md` sind deshalb eigene.
- Wer ein eigenes Projekt hier aufgeführt haben möchte oder einen Eintrag korrigieren will, öffnet ein Issue oder einen Pull Request.
