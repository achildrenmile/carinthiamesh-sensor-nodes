#ifdef EXAMPLE_leak_sensor
#include "fixture.h"
static const int PIN = 5;   // LEAK_PIN-Default; Modul zieht bei Naesse auf LOW (LEAK_WET_LEVEL)
static void dry() { pin_set(PIN, HIGH); }
static void wet() { pin_set(PIN, LOW); }

TEST(leak_sends_high_priority_alert) {
  dry(); Fixture f; f.start(); f.login();
  wet(); f.w.runSeconds(30);
  CHECK_EQ(f.fresh("Wasser erkannt!"), 1);
}

TEST(leak_short_splash_is_ignored) {
  dry(); Fixture f; f.start(); f.login();
  wet(); f.w.run(1000); dry();          // 1 s < 2 s Entprellzeit
  f.w.runSeconds(30);
  CHECK_EQ(f.alerts(), 0);
}

TEST(leak_alert_repeats_until_acked) {
  dry(); Fixture f; f.start(); f.login();
  f.phone->auto_ack = false;
  wet(); f.w.runSeconds(300);
  int n = f.count("Wasser erkannt!");
  printf("    ohne ACK: %d Zustellungen in 300 s\n", n);
  CHECK(n >= 2);
}

TEST(leak_stops_repeating_when_dry_again) {
  dry(); Fixture f; f.start(); f.login();
  f.phone->auto_ack = false;
  wet(); f.w.runSeconds(60);
  dry(); f.w.runSeconds(10);
  int n = f.count("Wasser erkannt!");
  f.w.runSeconds(300);
  CHECK_EQ(f.count("Wasser erkannt!"), n);   // nach dem Trocknen keine weiteren Wiederholungen
}

TEST(leak_rearm_waits_ten_minutes) {
  dry(); Fixture f; f.start(); f.login();
  wet(); f.w.runSeconds(30);
  dry(); f.w.runSeconds(10);
  wet(); f.w.runSeconds(120);            // zweite Naesse nach 40 s: noch in der Ruhezeit
  CHECK_EQ(f.fresh("Wasser erkannt!"), 1);
  dry(); f.w.runSeconds(600);
  wet(); f.w.runSeconds(30);             // jetzt >10 min spaeter
  CHECK_EQ(f.fresh("Wasser erkannt!"), 2);
}

TEST(leak_status_command) {
  dry(); Fixture f; f.start(); f.login();
  f.phone->sendCommand("status"); f.w.runSeconds(20);
  CHECK(!f.phone->cli_replies.empty());
  if (!f.phone->cli_replies.empty()) CHECK(f.phone->cli_replies.back() == "trocken");
}
#endif
