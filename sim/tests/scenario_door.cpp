#ifdef EXAMPLE_door_sensor
#include "fixture.h"
static const int PIN = 4;   // DOOR_PIN-Default
static void closed() { pin_set(PIN, LOW); }
static void open_() { pin_set(PIN, HIGH); }

TEST(door_open_sends_alert) {
  closed(); Fixture f; f.start(); f.login();
  CHECK_EQ(f.alerts(), 0);
  open_(); f.w.runSeconds(30);
  CHECK_EQ(f.count("Tuer offen"), 1);
}

TEST(door_close_sends_alert_after_cooldown) {
  closed(); Fixture f; f.start(); f.login();
  open_(); f.w.runSeconds(65);
  closed(); f.w.runSeconds(30);
  CHECK_EQ(f.count("Tuer offen"), 1);
  CHECK_EQ(f.count("Tuer geschlossen"), 1);
}

TEST(door_cooldown_blocks_second_message) {
  closed(); Fixture f; f.start(); f.login();
  open_(); f.w.runSeconds(5);
  closed(); f.w.runSeconds(20);     // innerhalb von 60 s: keine zweite Meldung
  CHECK_EQ(f.alerts(), 1);
}

TEST(door_bounce_is_debounced) {
  closed(); Fixture f; f.start(); f.login();
  for (int i = 0; i < 6; i++) { open_(); f.w.run(40); closed(); f.w.run(40); }   // Prellen, kuerzer als 200 ms
  f.w.runSeconds(10);
  CHECK_EQ(f.alerts(), 0);
}

TEST(door_already_open_at_boot_sends_nothing) {
  open_(); Fixture f; f.start(); f.login();
  f.w.runSeconds(60);
  CHECK_EQ(f.alerts(), 0);
}

TEST(door_hourly_budget_caps_messages) {
  closed(); Fixture f; f.w.step_ms = 5; f.start(); f.login();
  for (int i = 0; i < 57; i++) {            // alle 61 s umschalten, bleibt innerhalb der ersten Stunde
    if (i % 2 == 0) open_(); else closed();
    f.w.runSeconds(61);
  }
  printf("    1 Stunde Dauerschalten: %zu Meldungen angekommen\n", f.alerts());
  CHECK(f.alerts() <= 20);
  CHECK(f.alerts() >= 15);
}

TEST(door_status_command) {
  closed(); Fixture f; f.start(); f.login();
  f.phone->sendCommand("status"); f.w.runSeconds(20);
  CHECK(!f.phone->cli_replies.empty());
  if (!f.phone->cli_replies.empty()) CHECK(f.phone->cli_replies.back() == "Tuer geschlossen");
}
#endif
