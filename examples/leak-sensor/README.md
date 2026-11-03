# leak-sensor

Ein Wassermelder für Keller, Waschmaschine oder Heizungsraum. Ein Feuchtesensor-Modul mit Digital-Ausgang liefert den Pegel, der Heltec V3 schickt bei Nässe einen Alert mit hoher Priorität. Der wird wiederholt, bis ein Companion den Empfang bestätigt.

![Schaltplan](schaltplan.svg)

| | |
| --- | --- |
| Hardware | Heltec V3, Wassersensor-Modul mit Comparator (DO-Ausgang) |
| Build-Target | `Heltec_v3_sensor` |
| Meldet | „Wasser erkannt!“ als High-Priority-Alert per DM |
| Steuerbar | `status` in der Admin-Konsole: „nass“ oder „trocken“ |
| Airtime-Schutz | Wasser muss 2 s anliegen, nach einer Meldung 10 min Ruhe, max. 6 pro Stunde |
| Getestet gegen | MeshCore `main` 81dbae5 (v1.18.0), `pio run -e Heltec_v3_sensor` erfolgreich (3. Nov. 2026) |
| Status | kompiliert, nicht auf Hardware getestet |

## Dateien

| Datei | Inhalt |
| --- | --- |
| [`BOM.md`](BOM.md) | Stückliste |
| [`anleitung.md`](anleitung.md) | Verdrahtung, Build, Test |
| [`schaltplan.svg`](schaltplan.svg) | Schaltplan |
| [`firmware/main.cpp`](firmware/main.cpp) | fertige `main.cpp` für `examples/simple_sensor` |

## Verdrahtung

| Modul | Heltec-V3-Pin |
| --- | --- |
| VCC | 3V3 |
| GND | GND |
| DO | GPIO 5 |

## Anderes Board

Die Pins oben gelten für den Heltec V3. Auf anderem Board:

1. Ein Target mit `examples/simple_sensor` verwenden. Eines gibt es für viele Boards schon, z. B. `RAK_4631_sensor`, sonst im Varianten-`platformio.ini` eines anlegen.
2. Freie GPIOs mit 3,3 V Logik wählen und per Build-Flag setzen: `LEAK_PIN`, `LEAK_WET_LEVEL` (`LOW` oder `HIGH`). Beispiel: `-D LEAK_PIN=17 -D LEAK_WET_LEVEL=HIGH`
3. Pegel und Versorgung prüfen, die meisten Boards vertragen 3,3 V, nicht 5 V.
4. Den Aufbau mit dem Test aus der [Anleitung](anleitung.md) prüfen.

Kompiliert wurde mit den Heltec-Defaults für `Heltec_v3_sensor` sowie mit überschriebenen Pins für `RAK_4631_sensor` und `Xiao_nrf52_sensor`. Xiao nRF52840: Target aus [`targets/xiao_nrf52_sensor.ini`](../../targets/xiao_nrf52_sensor.ini) anlegen, Pins: `-D LEAK_PIN=D0`. Funktioniert hat es auf keinem Board in echt.

Allgemeine Hinweise zu Sensor-Nodes und Airtime stehen im [Haupt-README](../../README.md).
