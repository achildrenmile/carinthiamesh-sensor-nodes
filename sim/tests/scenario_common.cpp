// Szenarien, die fuer jedes Beispiel gelten: Anmeldung, Akku-Alert, Mesh-Verhalten.
#include "fixture.h"

TEST(login_with_password) {
  Fixture f; f.start(); f.login();
  CHECK(f.phone->logged_in);
}

TEST(login_with_wrong_password_is_rejected) {
  Fixture f; f.start();
  f.phone->login("falsch"); f.w.runSeconds(20);
  CHECK(!f.phone->logged_in);
}

TEST(no_alerts_without_battery) {   // nur USB: keine Akku-Alarme
  board.batt_mv = 0;
  Fixture f; f.start(); f.login();
  f.w.runSeconds(120);
  CHECK_EQ(f.count("Battery is critical!"), 0);
  CHECK_EQ(f.count("Battery is low"), 0);
}

// BEFUND: Der erste High-Priority-Alert kommt per Flood. Der Companion bestaetigt dann mit einer
// Pfad-Rueckmeldung, die einen ACK enthaelt (wie BaseChatMesh). SensorMesh::onPeerPathRecv speichert
// nur den Pfad und wertet diesen ACK nicht aus. Nach 8 s wiederholt der Sensor deshalb einmal,
// jetzt direkt, und der normale ACK stoppt die Wiederholung. Ergebnis: genau eine Doublette.
TEST(critical_battery_alert_stops_after_ack) {
  Fixture f; f.start(); f.login();
  board.batt_mv = 3300;
  f.w.runSeconds(120);
  int n = f.count("Battery is critical!");
  printf("    Zustellungen mit ACK: %d (1 erwartet, 2 durch den Befund oben)\n", n);
  CHECK(n >= 1 && n <= 2);
  CHECK_EQ(f.count("Battery is low"), 1);     // Low-Priority: ein Versuch
  f.w.runSeconds(300);
  CHECK_EQ(f.count("Battery is critical!"), n);   // danach nichts mehr
}

TEST(critical_battery_alert_is_repeated_without_ack) {
  Fixture f; f.start(); f.login();
  f.phone->auto_ack = false;
  board.batt_mv = 3300;
  f.w.runSeconds(300);
  int n = f.count("Battery is critical!");
  printf("    ohne ACK: %d Zustellungen in 300 s\n", n);
  CHECK(n >= 2);      // High-Priority wiederholt bis zum ACK
  CHECK(n <= 8);      // ... aber nicht endlos
}

TEST(alert_reaches_phone_over_repeater) {
  Fixture f; f.start(true); f.login();
  CHECK(f.phone->logged_in);
  board.batt_mv = 3300;
  f.w.runSeconds(180);
  CHECK(f.count("Battery is critical!") >= 1);
  CHECK(f.rep->forwarded > 0);
}

TEST(critical_alert_survives_lossy_link) {
  Fixture f; f.start(false, 0.4f);
  f.login();
  CHECK(f.phone->logged_in);
  board.batt_mv = 3300;
  f.w.runSeconds(600);
  CHECK(f.count("Battery is critical!") >= 1);
}

TEST(airtime_of_one_alert_is_small) {
  Fixture f; f.start(); f.login();
  uint32_t sent0 = radio_driver.packets_sent; uint64_t air0 = radio_driver.tx_airtime_ms;
  board.batt_mv = 3300;
  f.w.runSeconds(60);
  uint32_t pk = radio_driver.packets_sent - sent0; uint64_t air = radio_driver.tx_airtime_ms - air0;
  printf("    Sensor sendete %u Pakete, %llu ms Airtime (SF%d, BW %.1f)\n", pk, (unsigned long long)air, radio_driver.sf, radio_driver.bw);
  CHECK(pk >= 1);
  CHECK(air < 10000);
}
