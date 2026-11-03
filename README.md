# meshcore-button-sensor

Ein Sensor- und Aktor-Node für [MeshCore](https://github.com/meshcore-dev/MeshCore): vier Taster, eine LED und ein Buzzer an einem Heltec V3. Ein Tastendruck schickt eine Direktnachricht an angemeldete Companions (App oder T-Deck), LED und Buzzer lassen sich aus der App fernsteuern.

*English: A MeshCore sensor/actuator node example for the Heltec V3 with four buttons, an LED and a buzzer. Button presses are sent as rate-limited direct-message alerts; the LED and buzzer can be switched remotely via custom admin commands. Documentation is in German.*

## Was es kann

- 4 Taster an GPIO 4–7, entprellt, nur das Drücken wird gemeldet
- Meldung als Low-Priority-Alert per Direktnachricht, kein Public-Kanal
- Airtime-Schutz: max. 1 Meldung pro Taster alle 30 s, max. 20 pro Stunde
- LED (GPIO 41) und aktiver Buzzer (GPIO 42) als lokale Rückmeldung
- Fernsteuerung über die Admin-Konsole: `led on`, `led off`, `beep`
- Akku-Alarme der Vorlage nur, wenn wirklich ein Akku dran ist

Basis ist das offizielle Beispiel `examples/simple_sensor`. Geändert wird nur dessen `main.cpp`.

## Inhalt

| Datei | Inhalt |
| --- | --- |
| [`docs/anleitung.md`](docs/anleitung.md) | Schritt-für-Schritt-Anleitung: Verdrahtung, Build, Code, Test, Spielregeln |
| [`firmware/main.cpp`](firmware/main.cpp) | fertige `main.cpp` für `examples/simple_sensor` |
| [`firmware/simple_sensor-buttons.patch`](firmware/simple_sensor-buttons.patch) | dieselben Änderungen als Patch für `git apply` |

## Schnellstart

```bash
git clone https://github.com/meshcore-dev/MeshCore.git
git clone https://github.com/achildrenmile/meshcore-button-sensor.git
cd MeshCore
cp ../meshcore-button-sensor/firmware/main.cpp examples/simple_sensor/main.cpp
# ADVERT_NAME und ADMIN_PASSWORD in variants/heltec_v3/platformio.ini [env:Heltec_v3_sensor] anpassen
pio run -e Heltec_v3_sensor -t upload
```

Danach im Companion mit dem Admin-Passwort am Sensor anmelden und einen Taster drücken. Details in der [Anleitung](docs/anleitung.md).

## Verdrahtung

| Bauteil | Heltec-V3-Pin | Anschluss |
| --- | --- | --- |
| Taster 1–4 | GPIO 4, 5, 6, 7 | andere Seite an GND (interne Pull-ups) |
| LED | GPIO 41 | über 220–330 Ω an GND |
| Aktiver Buzzer (3,3 V) | GPIO 42 | Minus an GND |

## Bitte rücksichtsvoll senden

Jede Nachricht kostet Airtime auf allen Repeatern im Pfad, und ohne bekannten Pfad geht sie per Flood durchs ganze Netz. Cooldown und Stundenbudget daher bitte nicht entfernen, keine Meldungen in Public-Kanäle umleiten und neue Nodes in der lokalen Community ankündigen.

## Status

Geschrieben gegen MeshCore `main` 81dbae5 (v1.18.0, 15.10.2026). Der Code ist noch nicht auf Hardware getestet. Rückmeldungen und Pull Requests sind willkommen.

## Lizenz

MIT, siehe [LICENSE](LICENSE). `firmware/main.cpp` basiert auf dem MeshCore-Beispiel `simple_sensor` (MIT, Scott Powell / rippleradios.com).
