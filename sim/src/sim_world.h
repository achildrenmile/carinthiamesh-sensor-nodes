#pragma once
// Simulationswelt: Repeater, Companion und der Sensor unter Test auf einem virtuellen Funkmedium.
#include <Arduino.h>
#include <Mesh.h>
#include <helpers/ArduinoHelpers.h>
#include <helpers/StaticPoolPacketManager.h>
#include <helpers/SimpleMeshTables.h>
#include <helpers/TxtDataHelpers.h>
#include <helpers/AdvertDataHelpers.h>
#include <Identity.h>
#include <Packet.h>
#include <vector>
#include <string>
#include <functional>
#include "sim_radio.h"

extern sim::Medium g_medium;
#ifndef RESP_SERVER_LOGIN_OK
  #define RESP_SERVER_LOGIN_OK 0   // Antwort auf ANON_REQ, wie in MeshCore
#endif

namespace sim {

class SimMillis : public mesh::MillisecondClock { public: unsigned long getMillis() override { return millis(); } };

class SimRNG : public mesh::RNG {
  uint64_t s;
public:
  explicit SimRNG(uint64_t seed = 1) : s(seed * 2654435761u + 88172645463325252ull) {}
  void random(uint8_t* dest, size_t n) override {
    for (size_t i = 0; i < n; i++) { s ^= s << 13; s ^= s >> 7; s ^= s << 17; dest[i] = (uint8_t)(s >> 24); }
  }
};

// Gemeinsame Bausteine eines Nodes. Eigene Basisklasse, damit sie VOR mesh::Mesh gebaut werden
// (Dispatcher ruft im Konstruktor bereits die Uhr auf).
struct NodeParts {
  SimRadio radio; SimMillis ms; SimRNG rng; VolatileRTCClock rtc; StaticPoolPacketManager mgr; SimpleMeshTables tables;
  NodeParts(const char* name, uint64_t seed) : radio(g_medium, name), rng(seed), mgr(32) {}
};

// Repeater: leitet alles weiter (Flood einmal, Direct entlang des Pfads), sonst nichts.
class SimRepeater : public NodeParts, public mesh::Mesh {
public:
  uint32_t forwarded = 0;
  explicit SimRepeater(const char* name, uint64_t seed)
    : NodeParts(name, seed), mesh::Mesh(radio, ms, rng, rtc, mgr, tables) {}
  bool allowPacketForward(const mesh::Packet*) override { forwarded++; return true; }
  uint32_t getRetransmitDelay(const mesh::Packet*) override { return 200; }
  uint32_t getDirectRetransmitDelay(const mesh::Packet*) override { return 100; }
  void tick() { rtc.tick(); loop(); }
};

struct ReceivedMsg { std::string text; uint32_t at_ms; uint8_t attempt; bool was_flood; };

// Companion-Stand-in: meldet sich per Passwort am Sensor an, empfaengt Alert-DMs, bestaetigt per ACK.
class SimCompanion : public NodeParts, public mesh::Mesh {
public:
  mesh::Identity sensor_id; uint8_t secret[PUB_KEY_SIZE]; bool know_sensor = false;
  bool logged_in = false; bool auto_ack = true;
  std::vector<ReceivedMsg> inbox; uint32_t acks_sent = 0; uint32_t adverts_seen = 0;
  std::vector<std::string> cli_replies;

  explicit SimCompanion(const char* name, uint64_t seed)
    : NodeParts(name, seed), mesh::Mesh(radio, ms, rng, rtc, mgr, tables) { self_id = mesh::LocalIdentity(&rng); }

  void setSensor(const mesh::Identity& id) {
    sensor_id = id; know_sensor = true;
    self_id.calcSharedSecret(secret, id);
  }
  void tick() { rtc.tick(); loop(); }

  void login(const char* password) {
    uint8_t data[64]; uint32_t ts = rtc.getCurrentTimeUnique() + 1000;   // neuer als zuletzt gesehen
    memcpy(data, &ts, 4); size_t n = strlen(password); memcpy(&data[4], password, n + 1);
    auto pkt = createAnonDatagram(PAYLOAD_TYPE_ANON_REQ, self_id, sensor_id, secret, data, 4 + n + 1);
    if (pkt) sendFlood(pkt);
  }
  // Admin-Befehl (Textnachricht vom Typ CLI) an den Sensor
  void sendCommand(const char* cmd) {
    uint8_t data[MAX_PACKET_PAYLOAD]; uint32_t ts = rtc.getCurrentTimeUnique() + 2000 + (++cmd_seq);
    memcpy(data, &ts, 4); data[4] = (TXT_TYPE_CLI_DATA << 2);
    size_t n = strlen(cmd); memcpy(&data[5], cmd, n);
    auto pkt = createDatagram(PAYLOAD_TYPE_TXT_MSG, sensor_id, secret, data, 5 + n);
    if (pkt) sendFlood(pkt);
  }
protected:
  uint32_t cmd_seq = 0;
  bool allowPacketForward(const mesh::Packet*) override { return false; }   // Companion ist kein Repeater
  int searchPeersByHash(const uint8_t* hash) override { return (know_sensor && hash[0] == sensor_id.pub_key[0]) ? 1 : 0; }   // Anzahl passender Kontakte
  void getPeerSharedSecret(uint8_t* dest, int) override { memcpy(dest, secret, PUB_KEY_SIZE); }
  void onAdvertRecv(mesh::Packet*, const mesh::Identity&, uint32_t, const uint8_t*, size_t) override { adverts_seen++; }

  void onPeerDataRecv(mesh::Packet* packet, uint8_t type, int, const uint8_t* sec, uint8_t* data, size_t len) override {
    if (type == PAYLOAD_TYPE_RESPONSE) { if (len > 4 && data[4] == RESP_SERVER_LOGIN_OK) logged_in = true; return; }
    if (type != PAYLOAD_TYPE_TXT_MSG || len < 5) return;
    uint8_t flags = data[4] >> 2, attempt = data[4] & 3;
    data[len] = 0;
    if (flags == TXT_TYPE_CLI_DATA) { cli_replies.push_back((char*)&data[5]); return; }
    if (flags != TXT_TYPE_PLAIN) return;
    inbox.push_back({(char*)&data[5], millis(), attempt, packet->isRouteFlood()});
    if (!auto_ack) return;
    uint32_t ack;   // Hash aus Zeitstempel + Text + Absender-Schluessel, wie vom Sensor erwartet
    mesh::Utils::sha256((uint8_t*)&ack, 4, data, 5 + strlen((char*)&data[5]), sensor_id.pub_key, PUB_KEY_SIZE);
    acks_sent++;
    if (packet->isRouteFlood()) {
      auto path = createPathReturn(sensor_id, sec, packet->path, packet->path_len, PAYLOAD_TYPE_ACK, (uint8_t*)&ack, 4);
      if (path) sendFlood(path, 300);
    } else {
      auto a = createAck(ack);
      if (a) sendFlood(a, 300);
    }
  }
  bool onPeerPathRecv(mesh::Packet*, int, const uint8_t*, uint8_t*, uint8_t, uint8_t extra_type, uint8_t* extra, uint8_t extra_len) override {
    if (extra_type == PAYLOAD_TYPE_RESPONSE && extra_len > 4 && extra[4] == RESP_SERVER_LOGIN_OK) logged_in = true;
    return false;
  }
};

// Die Welt: taktet alle Teilnehmer mit 1 ms Schritten.
struct World {
  std::vector<std::function<void()>> tickers;
  uint32_t step_ms = 1;   // gröber (z. B. 5) beschleunigt lange Läufe
  void add(std::function<void()> f) { tickers.push_back(f); }
  void run(uint32_t ms) {
    for (uint32_t i = 0; i < ms; i += step_ms) {
      sim::now_ms += step_ms;
      for (auto& t : tickers) t();
      g_medium.step(sim::now_ms);
    }
  }
  void runSeconds(uint32_t s) { run(s * 1000); }
};

}  // namespace sim
