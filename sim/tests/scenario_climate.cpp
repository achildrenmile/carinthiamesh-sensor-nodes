#ifdef EXAMPLE_climate_sensor
#include "fixture.h"

TEST(climate_frost_alert_with_hysteresis) {
  sensors.bme280 = true; sensors.temp_c = 10; sensors.humidity = 50;
  Fixture f; f.start(); f.login();
  f.w.runSeconds(60);
  CHECK_EQ(f.count("Frostgefahr"), 0);
  sensors.temp_c = 1.0f; f.w.runSeconds(60);
  CHECK_EQ(f.count("Frostgefahr"), 1);
  sensors.temp_c = 3.0f; f.w.runSeconds(120);      // zwischen 2 und 4 Grad: Zustand bleibt, keine neue Meldung
  CHECK_EQ(f.count("Frostgefahr"), 1);
  sensors.temp_c = 6.0f; f.w.runSeconds(60);       // ueber 4 Grad: zurueckgesetzt
  sensors.temp_c = 0.5f; f.w.runSeconds(60);       // wieder Frost: neue Meldung
  CHECK_EQ(f.count("Frostgefahr"), 2);
}

// Regression: SensorMesh::getRelativeHumidity() liefert wegen falscher Feldgroesse Unsinn (siehe main.cpp).
// Mit dem eigenen Leser darf 50 % keinen Alarm ausloesen und 75 % muss einen ausloesen.
TEST(climate_humidity_alert_with_hysteresis) {
  sensors.bme280 = true; sensors.temp_c = 20; sensors.humidity = 50;
  Fixture f; f.start(); f.login();
  f.w.runSeconds(120);
  CHECK_EQ(f.count("Luftfeuchte hoch"), 0);
  sensors.humidity = 75; f.w.runSeconds(120);
  CHECK_EQ(f.count("Luftfeuchte hoch"), 1);
  sensors.humidity = 68; f.w.runSeconds(120);      // zwischen 65 und 70: keine neue Meldung
  CHECK_EQ(f.count("Luftfeuchte hoch"), 1);
}

TEST(climate_no_sensor_no_alerts) {
  sensors.bme280 = false;
  Fixture f; f.start(); f.login();
  f.w.runSeconds(180);
  CHECK_EQ(f.count("Frostgefahr"), 0);
  CHECK_EQ(f.count("Luftfeuchte hoch"), 0);
}

// Im Ruhezustand sendet der Sensor nur seine periodische Advert (Standard: alle 2 Minuten, Zero-Hop).
TEST(climate_idle_sends_only_adverts) {
  sensors.bme280 = true; sensors.temp_c = 20; sensors.humidity = 50;
  Fixture f; f.start(); f.login();
  uint32_t before = radio_driver.packets_sent;
  f.w.runSeconds(600);
  uint32_t n = radio_driver.packets_sent - before;
  printf("    Leerlauf: %u Pakete in 600 s (Adverts)\n", n);
  CHECK(n <= 6);                                   // Messwerte nur auf Abfrage, nichts ungefragt
}
#endif
