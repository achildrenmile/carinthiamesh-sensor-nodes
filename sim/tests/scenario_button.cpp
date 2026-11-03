#ifdef EXAMPLE_button_sensor
#include "fixture.h"
static const int BTN[4] = {4, 5, 6, 7};
static const int LED = 41, BUZ = 42;
static void release_all() { for (int p : BTN) pin_set(p, HIGH); }
static void press(int i, Fixture& f, uint32_t hold_ms = 300) { pin_set(BTN[i], LOW); f.w.run(hold_ms); pin_set(BTN[i], HIGH); f.w.run(100); }

TEST(button_press_sends_alert) {
  release_all(); Fixture f; f.start(); f.login();
  press(0, f); f.w.runSeconds(20);
  CHECK_EQ(f.count("Taster 1 gedrueckt"), 1);
}

TEST(button_each_has_own_text) {
  release_all(); Fixture f; f.start(); f.login();
  for (int i = 0; i < 4; i++) { press(i, f); f.w.runSeconds(15); }
  for (int i = 0; i < 4; i++) {
    char t[32]; snprintf(t, sizeof t, "Taster %d gedrueckt", i + 1);
    CHECK_EQ(f.count(t), 1);
  }
}

TEST(button_cooldown_per_button) {
  release_all(); Fixture f; f.start(); f.login();
  press(0, f); f.w.runSeconds(2);
  press(0, f); f.w.runSeconds(2);        // innerhalb von 30 s: nicht erneut gemeldet
  CHECK_EQ(f.count("Taster 1 gedrueckt"), 1);
  f.w.runSeconds(35);
  press(0, f); f.w.runSeconds(15);
  CHECK_EQ(f.count("Taster 1 gedrueckt"), 2);
}

TEST(button_local_feedback_costs_no_airtime) {
  release_all(); Fixture f; f.start(); f.login();
  press(0, f); f.w.runSeconds(15);
  uint32_t sent = radio_driver.packets_sent;
  uint32_t led0 = sim::pin_out_changes[LED];
  press(0, f);                           // im Cooldown: nur LED/Buzzer
  f.w.runSeconds(2);
  CHECK(sim::pin_out_changes[LED] > led0);
  CHECK_EQ(radio_driver.packets_sent, sent);
}

TEST(button_bounce_is_debounced) {
  release_all(); Fixture f; f.start(); f.login();
  for (int i = 0; i < 6; i++) { pin_set(BTN[0], LOW); f.w.run(10); pin_set(BTN[0], HIGH); f.w.run(10); }   // < 50 ms
  f.w.runSeconds(20);
  CHECK_EQ(f.alerts(), 0);
}

TEST(button_hourly_budget_caps_messages) {
  release_all(); Fixture f; f.w.step_ms = 5; f.start(); f.login();
  for (int i = 0; i < 100; i++) { press(i % 4, f); f.w.runSeconds(35); }   // alle 35 s, passiert den Cooldown
  printf("    ~58 Minuten Dauerdruecken: %zu Meldungen angekommen\n", f.alerts());
  CHECK(f.alerts() <= 20 + 2);
  CHECK(f.alerts() >= 15);
}

TEST(button_led_and_beep_commands) {
  release_all(); Fixture f; f.start(); f.login();
  f.phone->sendCommand("led on"); f.w.runSeconds(20);
  CHECK_EQ(sim::pin_out[LED], HIGH);
  f.phone->sendCommand("led off"); f.w.runSeconds(20);
  CHECK_EQ(sim::pin_out[LED], LOW);
  uint32_t b0 = sim::pin_out_changes[BUZ];
  f.phone->sendCommand("beep"); f.w.runSeconds(20);
  CHECK(sim::pin_out_changes[BUZ] >= b0 + 2);   // an und wieder aus
  CHECK_EQ(sim::pin_out[BUZ], LOW);
}
#endif
