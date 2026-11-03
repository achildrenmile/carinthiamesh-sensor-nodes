# Simulation

Läuft die echte Firmware-Logik eines Beispiels auf dem PC, bevor etwas gelötet wird. Ohne Hardware, ohne Funk, in wenigen Sekunden.

**Was echt ist:** die `MyMesh`-Klasse aus `examples/<name>/firmware/main.cpp` (unverändert), MeshCore selbst (`SensorMesh`, Mesh-Protokoll, Verschlüsselung, Anmeldung, Alerts, Wiederholung bis zum ACK) und die Crypto-Bibliothek.

**Was simuliert ist:** Funkmodul und Funkstrecke, Board, Dateisystem, Pins (Taster, Kontakte), Sensorwerte (Akku, BME280) sowie Companion-App und Repeater.

## Ausführen

```bash
git clone https://github.com/meshcore-dev/MeshCore.git
git -C MeshCore checkout 81dbae5
pip install platformio
(cd MeshCore && pio pkg install -e Heltec_v3_sensor)     # holt Crypto und CayenneLPP
make -C sim test MESHCORE=$PWD/MeshCore
```

Voraussetzung: `g++` (C++17) und `make`. Ein Beispiel allein: `make -C sim test EXAMPLES=door-sensor MESHCORE=…`.

## Was die Tests prüfen

| Bereich | Tests |
| --- | --- |
| alle Beispiele | Anmeldung mit richtigem und falschem Passwort, Akku-Alerts, keine Akku-Alarme ohne Akku, Alert über Repeater, Alert bei 40 % Paketverlust, Airtime einer Meldung, Wiederholung ohne ACK und kein Dauerfeuer |
| door-sensor | Auf, Zu, Cooldown, Prellen, Start im offenen Zustand, 20-pro-Stunde-Grenze, `status` |
| leak-sensor | Alarm erst nach 2 s Nässe, kurze Spritzer ignoriert, Wiederholung bis ACK, Ende der Wiederholung bei Trockenheit, 10 Minuten Ruhezeit |
| climate-sensor | Frost und Feuchte mit Hysterese, ohne Sensor keine Alarme, im Leerlauf nur Adverts |
| button-sensor | vier Taster, Cooldown je Taster, LED/Buzzer lokal ohne Airtime, Stundenbudget, `led on/off`, `beep` |

## Was die Simulation nicht beweist

- Elektrik: Pegel, Prellen echter Kontakte, Strom, Verdrahtung.
- Funkphysik: Dämpfung, Reichweite, Störungen. Das Medium kennt nur Verbindungen mit einstellbarer Verlustrate, Airtime nach der LoRa-Formel und Kollisionen.
- Die echte App: Der Companion ist ein Stand-in nach dem Vorbild von `BaseChatMesh`.
- Chip-Verhalten: gerechnet wird mit dem PC-Compiler, nicht mit ESP32 oder nRF52.

Ein grüner Lauf heißt: Die Logik tut, was sie soll. Ob der Aufbau funktioniert, zeigt erst Hardware.

## Aufbau

| Pfad | Inhalt |
| --- | --- |
| `mock/` | Attrappen für `Arduino.h`, Dateisystem, `RTClib.h`, `target.h` |
| `src/sim_radio.h` | virtuelles Funkmedium mit Airtime, Verlust, Kollisionen |
| `src/sim_world.h` | Repeater, Companion, Zeitgeber |
| `src/sim_sensor.h` | bindet die `MyMesh`-Klasse des Beispiels ein |
| `gen_app.sh` | schneidet den Anwendungsteil aus `main.cpp` (alles vor `StdRNG fast_rng;`) |
| `tests/` | Szenarien |

Jeder Testfall läuft in einem eigenen Prozess, damit die globalen Zustände sauber getrennt sind.

## Befunde

Beim Aufbau der Simulation sind zwei Dinge in MeshCore (Stand 81dbae5) aufgefallen. Beide sind bisher nur in der Simulation gesehen, nicht auf Hardware.

1. **`SensorMesh::getRelativeHumidity()` liest falsch.** CayenneLPP schreibt die Luftfeuchte mit 1 Byte (0,5 %). `LPPData::getDataSize()` rechnet mit 2 Byte und Faktor 10. Der gelesene Wert ist Unsinn, und der Eintrag dahinter (Luftdruck) verrutscht. Das `climate-sensor`-Beispiel nutzt deshalb einen eigenen Leser.
2. **Erster High-Priority-Alert kommt doppelt an.** Kommt der Alert per Flood, bestätigt der Companion mit einer Pfad-Rückmeldung, die den ACK enthält. `SensorMesh::onPeerPathRecv()` speichert nur den Pfad und wertet den ACK nicht aus (anders als `BaseChatMesh` und der Room Server). Nach 8 s wiederholt der Sensor einmal, dann direkt, und dieser ACK stoppt ihn.

Ob das so auf echter Hardware auftritt, ist offen. Für beides wäre ein Upstream-Issue sinnvoll.

## Airtime-Zahlen aus der Simulation

Mit den MeshCore-Standardwerten (SF8, 62,5 kHz):

- Ein Alert samt Anmeldung und ACK: wenige Pakete, unter 1 Sekunde Sendezeit des Sensors.
- Im Leerlauf sendet der Sensor etwa alle 2 Minuten eine Zero-Hop-Advert (Standard von MeshCore, 5 Pakete in 10 Minuten).
