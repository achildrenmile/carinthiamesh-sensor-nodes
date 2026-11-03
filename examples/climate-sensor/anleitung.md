# Anleitung climate-sensor

## 1. Aufbau

BME280 an 3V3, GND, GPIO 33 (SDA) und GPIO 34 (SCL) anschließen. Der Heltec-V3-Sensor-Build nutzt dafür den zweiten I2C-Bus (`ENV_PIN_SDA=33`, `ENV_PIN_SCL=34`). Die BME280-Unterstützung ist in `variants/heltec_v3/platformio.ini` über `sensor_base` schon eingeschaltet, es sind keine zusätzlichen Build-Flags nötig. Die Standard-Adresse ist 0x76. Hat das Breakout 0x77, im `[env:Heltec_v3_sensor]` `-D TELEM_BME280_ADDRESS=0x77` ergänzen.

## 2. Build

```bash
git clone https://github.com/meshcore-dev/MeshCore.git
git clone https://github.com/achildrenmile/carinthiamesh-sensor-nodes.git
cd MeshCore
cp ../carinthiamesh-sensor-nodes/examples/climate-sensor/firmware/main.cpp examples/simple_sensor/main.cpp
# ADVERT_NAME und ADMIN_PASSWORD in variants/heltec_v3/platformio.ini [env:Heltec_v3_sensor] anpassen
pio run -e Heltec_v3_sensor -t upload
```

## 3. Test

1. Im Companion mit dem Admin-Passwort am Sensor anmelden.
2. Telemetrie abfragen: Temperatur, Feuchte und Druck erscheinen.
3. Alert testen: Sensor kurz anhauchen, bis die Feuchte über 70 % steigt, oder `FROST_ON_C` testweise auf 30 setzen.

## 4. Anpassen

Schwellen oben in `main.cpp`. `ENV_CHANNEL` ist 2, weil der erste Umweltsensor nach dem Board (Kanal 1) auf Kanal 2 liegt. Werden weitere I2C-Sensoren erkannt, kann sich der Kanal verschieben.
