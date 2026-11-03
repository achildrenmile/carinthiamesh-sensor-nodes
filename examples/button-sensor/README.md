# button-sensor

Vier Taster, eine LED und ein Buzzer an einem Heltec V3. Ein Tastendruck schickt eine Direktnachricht an angemeldete Companions (App oder T-Deck). LED und Buzzer lassen sich aus der App fernsteuern, der Node ist damit Sensor und Aktor zugleich.

![Schaltplan](schaltplan.svg)

| | |
| --- | --- |
| Hardware | Heltec V3, 4 Taster, LED + 220–330 Ω, aktiver Buzzer 3,3 V |
| Build-Target | `Heltec_v3_sensor` |
| Meldet | „Taster N gedrueckt“ als Low-Priority-Alert per DM |
| Steuerbar | `led on`, `led off`, `beep` über die Admin-Konsole |
| Airtime-Schutz | 1 Meldung pro Taster alle 30 s, max. 20 pro Stunde |
| Getestet gegen | MeshCore `main` 81dbae5 (v1.18.0) |
| Status | kompiliert, nicht auf Hardware getestet |

## Dateien

| Datei | Inhalt |
| --- | --- |
| [`anleitung.md`](anleitung.md) | Verdrahtung, Breadboard, Build, Code, Test |
| [`schaltplan.svg`](schaltplan.svg) / [`.png`](schaltplan.png) | Schaltplan |
| [`firmware/main.cpp`](firmware/main.cpp) | fertige `main.cpp` für `examples/simple_sensor` |
| [`firmware/simple_sensor-buttons.patch`](firmware/simple_sensor-buttons.patch) | dieselben Änderungen als Patch für `git apply` |

## Schnellstart

```bash
git clone https://github.com/meshcore-dev/MeshCore.git
git clone https://github.com/achildrenmile/carinthiamesh-sensor-nodes.git
cd MeshCore
cp ../carinthiamesh-sensor-nodes/examples/button-sensor/firmware/main.cpp examples/simple_sensor/main.cpp
# ADVERT_NAME und ADMIN_PASSWORD in variants/heltec_v3/platformio.ini [env:Heltec_v3_sensor] anpassen
pio run -e Heltec_v3_sensor -t upload
```

Danach im Companion mit dem Admin-Passwort am Sensor anmelden und einen Taster drücken. Alles Weitere in der [Anleitung](anleitung.md).

## Verdrahtung

| Bauteil | Heltec-V3-Pin | Anschluss |
| --- | --- | --- |
| Taster 1–4 | GPIO 4, 5, 6, 7 | andere Seite an GND (interne Pull-ups) |
| LED | GPIO 41 | über 220–330 Ω an GND |
| Aktiver Buzzer (3,3 V) | GPIO 42 | Minus an GND |

## Anderes Board

Die Pins oben gelten für den Heltec V3. Auf anderem Board:

1. Ein Target mit `examples/simple_sensor` verwenden. Eines gibt es für viele Boards schon, z. B. `RAK_4631_sensor`, sonst im Varianten-`platformio.ini` eines anlegen.
2. Freie GPIOs mit 3,3 V Logik wählen und per Build-Flag setzen: `BTN1_PIN` … `BTN4_PIN`, `LED_PIN`, `BUZZER_PIN`. Beispiel: `-D BTN1_PIN=17 -D LED_PIN=21`
3. Pegel und Versorgung prüfen, die meisten Boards vertragen 3,3 V, nicht 5 V.
4. Den Aufbau mit dem Test aus der [Anleitung](anleitung.md) prüfen.

Kompiliert wurde mit den Heltec-Defaults für `Heltec_v3_sensor` sowie mit überschriebenen Pins für `RAK_4631_sensor` und `Xiao_nrf52_sensor`. Xiao nRF52840: Target aus [`targets/xiao_nrf52_sensor.ini`](../../targets/xiao_nrf52_sensor.ini) anlegen, Pins: `-D BTN1_PIN=D0 -D BTN2_PIN=D8 -D BTN3_PIN=D9 -D BTN4_PIN=D10 -D LED_PIN=D6 -D BUZZER_PIN=D7` (LED und Buzzer teilen sich die Pins mit dem I2C-Bus, deshalb nur ohne I2C-Sensor). Funktioniert hat es auf keinem Board in echt.

Allgemeine Hinweise zu Sensor-Nodes und Airtime stehen im [Haupt-README](../../README.md).
