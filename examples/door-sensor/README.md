# door-sensor

Ein Reed-Kontakt an einem Heltec V3 meldet, wenn eine Tür, ein Fenster oder ein Schaltschrank geöffnet oder geschlossen wird. Die Meldung kommt als Direktnachricht in die Companion-App.

![Schaltplan](schaltplan.svg)

| | |
| --- | --- |
| Hardware | Heltec V3, Reed-Kontakt mit Magnet |
| Build-Target | `Heltec_v3_sensor` |
| Meldet | „Tuer offen“ / „Tuer geschlossen“ als Low-Priority-Alert per DM |
| Steuerbar | `status` in der Admin-Konsole fragt den Zustand ab |
| Airtime-Schutz | Entprellen 200 ms, 1 Meldung pro Minute, max. 20 pro Stunde |
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

| Bauteil | Heltec-V3-Pin |
| --- | --- |
| Reed-Kontakt, Pin 1 | GPIO 4 (interner Pull-up) |
| Reed-Kontakt, Pin 2 | GND |

## Anderes Board

Die Pins oben gelten für den Heltec V3. Auf anderem Board:

1. Ein Target mit `examples/simple_sensor` verwenden. Eines gibt es für viele Boards schon, z. B. `RAK_4631_sensor`, sonst im Varianten-`platformio.ini` eines anlegen.
2. Freie GPIOs mit 3,3 V Logik wählen und per Build-Flag setzen: `DOOR_PIN`. Beispiel: `-D DOOR_PIN=17`
3. Pegel und Versorgung prüfen, die meisten Boards vertragen 3,3 V, nicht 5 V.
4. Den Aufbau mit dem Test aus der [Anleitung](anleitung.md) prüfen.

Kompiliert wurde mit den Heltec-Defaults für `Heltec_v3_sensor` sowie mit überschriebenen Pins für `RAK_4631_sensor` und `Xiao_nrf52_sensor`. Xiao nRF52840: Target aus [`targets/xiao_nrf52_sensor.ini`](../../targets/xiao_nrf52_sensor.ini) anlegen, Pins: `-D DOOR_PIN=D0`. Funktioniert hat es auf keinem Board in echt.

Allgemeine Hinweise zu Sensor-Nodes und Airtime stehen im [Haupt-README](../../README.md).
