# Anleitung leak-sensor

## 1. Aufbau

Modul an 3V3, GND und GPIO 5 (DO) anschließen. Die Fühlerplatte liegt flach am tiefsten Punkt, das Auswertemodul bleibt trocken.

## 2. Pegel prüfen

Die meisten LM393-Module ziehen DO bei Nässe auf LOW, einige auf HIGH. Fühlerplatte trocken und nass halten und die LED am Modul beobachten. Ist es umgekehrt, `LEAK_WET_LEVEL` in `main.cpp` auf `HIGH` setzen. Mit dem Poti am Modul stellt man die Empfindlichkeit ein.

## 3. Build

```bash
git clone https://github.com/meshcore-dev/MeshCore.git
git clone https://github.com/achildrenmile/carinthiamesh-sensor-nodes.git
cd MeshCore
cp ../carinthiamesh-sensor-nodes/examples/leak-sensor/firmware/main.cpp examples/simple_sensor/main.cpp
# ADVERT_NAME und ADMIN_PASSWORD in variants/heltec_v3/platformio.ini [env:Heltec_v3_sensor] anpassen
pio run -e Heltec_v3_sensor -t upload
```

## 4. Test

1. Im Companion mit dem Admin-Passwort am Sensor anmelden.
2. Fühlerplatte mit einem feuchten Tuch berühren: nach 2 s kommt „Wasser erkannt!“.
3. Der High-Priority-Alert wird wiederholt, bis die App ihn bestätigt.
4. `status` zeigt „nass“ oder „trocken“.

## 5. Hinweis

Ein Funk-Wassermelder ersetzt keinen Überflutungsschutz mit Absperrventil. Nicht auf Hardware getestet.
