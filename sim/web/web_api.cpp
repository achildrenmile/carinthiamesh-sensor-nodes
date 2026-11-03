// Schnittstelle der Simulation fuer den Browser (WebAssembly). Eine Instanz = ein Sensor-Beispiel,
// ein Companion ("Handy") und optional ein Repeater dazwischen. Alles Weitere steuert JavaScript.
#include "../src/sim_sensor.h"
#include <emscripten/emscripten.h>
#include <memory>
#include <string>
#include <sstream>

using namespace sim;

static std::unique_ptr<SensorNode> node;
static std::unique_ptr<SimCompanion> phone;
static std::unique_ptr<SimRepeater> rep;
static World world;
static size_t inbox_sent = 0, cli_sent = 0, ev_sent = 0;
static std::string out;

static const char* ptype(uint8_t header) {
  static const char* n[] = {"REQ", "RESPONSE", "TXT", "ACK", "ADVERT", "GRP_TXT", "GRP_DATA", "ANON_REQ", "PATH", "TRACE", "MULTIPART", "CONTROL", "?", "?", "?", "RAW"};
  return n[(header >> 2) & 0xF];
}
static std::string esc(const std::string& s) {
  std::string r;
  for (char c : s) { if (c == '"' || c == '\\') { r += '\\'; r += c; } else if ((unsigned char)c >= 32) r += c; }
  return r;
}

extern "C" {

EMSCRIPTEN_KEEPALIVE void sim_start(int use_repeater, int loss_pct) {
  g_medium.record_events = true;
  node.reset(new SensorNode());
  phone.reset(new SimCompanion("Handy", 3));
  phone->begin();
  phone->setSensor(node->mesh.self_id);
  float loss = loss_pct / 100.0f;
  if (use_repeater) {
    rep.reset(new SimRepeater("Repeater", 5)); rep->begin();
    g_medium.link(&radio_driver, &rep->radio, loss);
    g_medium.link(&rep->radio, &phone->radio, loss);
    world.add([] { rep->tick(); });
  } else {
    g_medium.link(&radio_driver, &phone->radio, loss);
  }
  world.add([] { node->tick(); });
  world.add([] { phone->tick(); });
  world.step_ms = 5;
  world.run(2000);
}
EMSCRIPTEN_KEEPALIVE void sim_run(int ms) { world.run((uint32_t)ms); }
EMSCRIPTEN_KEEPALIVE uint32_t sim_now() { return sim::now_ms; }
EMSCRIPTEN_KEEPALIVE void sim_login() { phone->login("password"); }
EMSCRIPTEN_KEEPALIVE void sim_command(const char* cmd) { phone->sendCommand(cmd); }
EMSCRIPTEN_KEEPALIVE void sim_auto_ack(int on) { phone->auto_ack = on != 0; }
EMSCRIPTEN_KEEPALIVE void sim_pin_set(int pin, int level) { pin_in_driven[pin] = true; pin_in[pin] = (uint8_t)level; }
EMSCRIPTEN_KEEPALIVE void sim_pin_release(int pin) { pin_in_driven[pin] = false; }
EMSCRIPTEN_KEEPALIVE int sim_pin_out(int pin) { return pin_out[pin]; }
EMSCRIPTEN_KEEPALIVE int sim_pin_mode(int pin) { return pin_mode[pin]; }
EMSCRIPTEN_KEEPALIVE void sim_env(int bme280, float t, float rh, float p) {
  sensors.bme280 = bme280 != 0; sensors.temp_c = t; sensors.humidity = rh; sensors.pressure_hpa = p;
}
EMSCRIPTEN_KEEPALIVE void sim_battery(int mv) { board.batt_mv = (uint16_t)mv; }

// Neue Ereignisse seit dem letzten Aufruf als JSON (Pakete auf der Luft, Nachrichten am Handy).
EMSCRIPTEN_KEEPALIVE const char* sim_events() {
  std::ostringstream o;
  o << "{\"now\":" << sim::now_ms << ",\"logged_in\":" << (phone->logged_in ? "true" : "false")
    << ",\"airtime_ms\":" << g_medium.total_airtime_ms << ",\"sensor_packets\":" << radio_driver.packets_sent
    << ",\"acks\":" << phone->acks_sent << ",\"packets\":[";
  for (size_t i = ev_sent; i < g_medium.events.size(); i++) {
    auto& e = g_medium.events[i];
    o << (i > ev_sent ? "," : "") << "{\"t\":" << e.t_start << ",\"end\":" << e.t_end << ",\"from\":\"" << esc(e.from)
      << "\",\"to\":\"" << esc(e.to) << "\",\"len\":" << e.len << ",\"type\":\"" << ptype(e.header)
      << "\",\"ok\":" << (e.delivered ? "true" : "false") << "}";
  }
  ev_sent = g_medium.events.size();
  o << "],\"messages\":[";
  for (size_t i = inbox_sent; i < phone->inbox.size(); i++) {
    auto& m = phone->inbox[i];
    o << (i > inbox_sent ? "," : "") << "{\"t\":" << m.at_ms << ",\"text\":\"" << esc(m.text) << "\",\"attempt\":" << (int)m.attempt << "}";
  }
  inbox_sent = phone->inbox.size();
  o << "],\"replies\":[";
  for (size_t i = cli_sent; i < phone->cli_replies.size(); i++)
    o << (i > cli_sent ? "," : "") << "\"" << esc(phone->cli_replies[i]) << "\"";
  cli_sent = phone->cli_replies.size();
  o << "]}";
  out = o.str();
  return out.c_str();
}

}  // extern "C"
