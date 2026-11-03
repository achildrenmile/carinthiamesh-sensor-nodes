#pragma once
// Gemeinsamer Aufbau fuer die Szenarien: Sensor, Companion (angemeldet), optional Repeater-Kette.
#include "../src/sim_sensor.h"
#include "harness.h"
#include <memory>
using namespace sim;

// Pins wie in der jeweiligen Beispiel-Firmware (Heltec-V3-Defaults)
inline void pin_set(int pin, int level) { sim::pin_in_driven[pin] = true; sim::pin_in[pin] = (uint8_t)level; }

struct Fixture {
  std::unique_ptr<SensorNode> node;
  std::unique_ptr<SimCompanion> phone;
  std::unique_ptr<SimRepeater> rep;       // optional
  World w;

  // Vor dem Start muessen die Eingaenge gesetzt sein, deshalb Setup in zwei Schritten.
  void start(bool via_repeater = false, float loss = 0.0f) {
    node.reset(new SensorNode());
    phone.reset(new SimCompanion("phone", 3));
    phone->begin();
    phone->setSensor(node->mesh.self_id);
    if (via_repeater) {
      rep.reset(new SimRepeater("rep", 5)); rep->begin();
      g_medium.link(&radio_driver, &rep->radio, loss);
      g_medium.link(&rep->radio, &phone->radio, loss);
      w.add([this] { rep->tick(); });
    } else {
      g_medium.link(&radio_driver, &phone->radio, loss);
    }
    w.add([this] { node->tick(); });
    w.add([this] { phone->tick(); });
    w.runSeconds(2);
  }
  void login() {
    for (int i = 0; i < 5 && !phone->logged_in; i++) { phone->login("password"); w.runSeconds(15); }
  }
  int count(const char* text) const {
    int n = 0; for (auto& m : phone->inbox) if (m.text == text) n++;
    return n;
  }
  // Zaehlt nur erste Zustellversuche (Attempt 0). High-Priority-Alerts koennen als Wiederholung doppelt ankommen.
  int fresh(const char* text) const {
    int n = 0; for (auto& m : phone->inbox) if (m.text == text && m.attempt == 0) n++;
    return n;
  }
  // Alle angekommenen Meldungen
  size_t alerts() const { return phone->inbox.size(); }
  uint32_t sensorPackets() const { return radio_driver.packets_sent; }
};
