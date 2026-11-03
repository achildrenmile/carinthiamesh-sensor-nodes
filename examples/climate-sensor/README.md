# climate-sensor

Ein BME280 am Heltec V3 misst Temperatur, Luftfeuchte und Luftdruck. Die Werte holt sich ein angemeldeter Companion per Telemetrie-Abfrage. Zusätzlich gibt es Alerts bei Frostgefahr und hoher Luftfeuchte, etwa für Keller, Gartenhaus oder Wochenendhaus.

![Schaltplan](schaltplan.svg)

| | |
| --- | --- |
| Hardware | Heltec V3, BME280-Breakout (I2C) |
| Build-Target | `Heltec_v3_sensor` (BME280 ist dort schon aktiviert, I2C auf GPIO 33/34) |
| Meldet | „Frostgefahr“ (unter 2 °C, zurück über 4 °C), „Luftfeuchte hoch“ (über 70 %, zurück unter 65 %) |
| Abfragbar | Telemetrie: Akku, Temperatur, Luftfeuchte, Luftdruck, Höhe; Min/Max/Mittel der letzten 24 h |
| Airtime-Schutz | Messwerte nur auf Abfrage; Alerts mit Hysterese, nur bei Zustandswechsel |
| Getestet gegen | nicht kompiliert; gebaut auf MeshCore `main` 81dbae5 (v1.18.0) |
| Status | nicht auf Hardware getestet |

## Dateien

| Datei | Inhalt |
| --- | --- |
| [`BOM.md`](BOM.md) | Stückliste |
| [`anleitung.md`](anleitung.md) | Verdrahtung, Build, Test |
| [`schaltplan.svg`](schaltplan.svg) | Schaltplan |
| [`firmware/main.cpp`](firmware/main.cpp) | fertige `main.cpp` für `examples/simple_sensor` |

## Verdrahtung

| BME280 | Heltec-V3-Pin |
| --- | --- |
| VCC | 3V3 |
| GND | GND |
| SDA | GPIO 33 |
| SCL | GPIO 34 |

Allgemeine Hinweise zu Sensor-Nodes und Airtime stehen im [Haupt-README](../../README.md).
