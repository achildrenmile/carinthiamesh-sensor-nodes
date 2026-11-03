# Anleitung door-sensor

## 1. Aufbau

Reed-Kontakt zwischen GPIO 4 und GND klemmen, den Magnet an die bewegliche Seite (Tür), den Kontakt an den Rahmen. Mit Magnet in der Nähe ist der Kontakt geschlossen (LOW = „zu“), ohne Magnet offen (HIGH = „offen“).

## 2. Build

```bash
git clone https://github.com/meshcore-dev/MeshCore.git
git clone https://github.com/achildrenmile/carinthiamesh-sensor-nodes.git
cd MeshCore
cp ../carinthiamesh-sensor-nodes/examples/door-sensor/firmware/main.cpp examples/simple_sensor/main.cpp
# ADVERT_NAME und ADMIN_PASSWORD in variants/heltec_v3/platformio.ini [env:Heltec_v3_sensor] anpassen
pio run -e Heltec_v3_sensor -t upload
```

## 3. Test

1. Im Companion mit dem Admin-Passwort am Sensor anmelden.
2. Magnet vom Kontakt entfernen: „Tuer offen“ kommt als DM.
3. Magnet zurück: „Tuer geschlossen“ (frühestens 60 s nach der ersten Meldung).
4. `status` in der Admin-Konsole liefert den aktuellen Zustand.

## 4. Anpassen

`COOLDOWN_MS`, `MAX_PER_HOUR` und `DEBOUNCE_MS` oben in `main.cpp`. Beim Start wird der aktuelle Zustand ohne Meldung übernommen.
