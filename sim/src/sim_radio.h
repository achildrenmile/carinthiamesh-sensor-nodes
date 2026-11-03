#pragma once
// Virtuelles Funkmedium: verbindet beliebig viele Nodes, rechnet Airtime nach der
// LoRa-Formel und verwirft Pakete bei Halbduplex-Konflikt, Kollision oder Verlust.
#include <Mesh.h>
#include <math.h>
#include <deque>
#include <vector>
#include <map>
#include <string>

namespace sim {

class SimRadio;

class Medium {
public:
  struct Tx { SimRadio* from; uint32_t start, end; std::vector<uint8_t> bytes; bool done = false; };
  std::vector<SimRadio*> radios;
  std::deque<Tx> txs;
  // Ereignisprotokoll fuer die Web-Oberflaeche (aus, solange nicht eingeschaltet)
  struct Event { uint32_t t_start, t_end; std::string from, to; int len; uint8_t header; bool delivered; };
  bool record_events = false;
  std::vector<Event> events;
  uint64_t total_airtime_ms = 0;     // Summe aller Sendezeiten im Netz
  uint32_t total_packets = 0;
  uint32_t rng_state = 12345;
  void add(SimRadio* r) { radios.push_back(r); }
  void link(SimRadio* a, SimRadio* b, float loss = 0.0f);   // beidseitig
  void unlink(SimRadio* a, SimRadio* b);
  bool hears(SimRadio* rx, SimRadio* tx) const;
  float lossOf(SimRadio* a, SimRadio* b) const;
  void step(uint32_t now);
  float rnd() { rng_state = rng_state * 1664525u + 1013904223u; return (rng_state >> 8) / 16777216.0f; }
private:
  std::map<std::pair<SimRadio*, SimRadio*>, float> links;
  friend class SimRadio;
};

class SimRadio : public mesh::Radio {
public:
  Medium* medium;
  std::string name;
  // LoRa-Parameter (MeshCore-Standard)
  float freq = 869.618f, bw = 62.5f; uint8_t sf = 8, cr = 5;
  int8_t tx_power = 14;
  // Statistik
  uint32_t packets_sent = 0, packets_recv = 0;
  uint64_t tx_airtime_ms = 0;
  std::vector<std::vector<uint8_t>> sent_log;   // gesendete Rohpakete (fuer Tests)

  explicit SimRadio(Medium& m, const char* n = "") : medium(&m), name(n) { m.add(this); }

  void setParams(float f, float b, uint8_t s, uint8_t c) { freq = f; bw = b; sf = s; cr = c; }
  void setTxPower(int8_t p) { tx_power = p; }
  bool setRxBoostedGainMode(bool) { return true; }
  bool getRxBoostedGainMode() const { return true; }
  float getLastRSSI() const override { return -80; }
  float getLastSNR() const override { return 5; }
  uint32_t getPacketsRecv() const { return packets_recv; }
  uint32_t getPacketsRecvErrors() const { return 0; }
  uint32_t getPacketsSent() const { return packets_sent; }

  uint32_t airtimeMs(int len) const {
    double tsym = pow(2.0, sf) / (bw * 1000.0) * 1000.0;               // ms
    bool de = tsym > 16.0;                                              // Low-Data-Rate-Optimierung
    double tpre = (8 + 4.25) * tsym;
    double num = 8.0 * len - 4.0 * sf + 28 + 16 - 0;                    // CRC an, expliziter Header
    double den = 4.0 * (sf - (de ? 2 : 0));
    double n = 8 + std::max(ceil(num / den) * cr, 0.0);
    return (uint32_t)ceil(tpre + n * tsym);
  }
  uint32_t getEstAirtimeFor(int len) override { return airtimeMs(len); }
  float packetScore(float, int) override { return 1.0f; }
  int recvRaw(uint8_t* bytes, int sz) override {
    if (rx.empty()) return 0;
    auto p = rx.front(); rx.pop_front();
    int n = std::min((int)p.size(), sz);
    memcpy(bytes, p.data(), n);
    packets_recv++;
    return n;
  }
  bool startSendRaw(const uint8_t* bytes, int len) override {
    Medium::Tx t; t.from = this; t.start = millis(); t.end = t.start + airtimeMs(len);
    t.bytes.assign(bytes, bytes + len);
    medium->txs.push_back(t);
    sending = true; tx_end = t.end;
    packets_sent++; tx_airtime_ms += (t.end - t.start);
    medium->total_airtime_ms += (t.end - t.start); medium->total_packets++;
    sent_log.emplace_back(bytes, bytes + len);
    return true;
  }
  bool isSendComplete() override { return !sending; }
  void onSendFinished() override {}
  bool isInRecvMode() const override { return !sending; }
  bool isReceiving() override { return false; }
  int getNoiseFloor() const override { return -110; }

  bool sending = false; uint32_t tx_end = 0;
  std::deque<std::vector<uint8_t>> rx;
};

inline void Medium::link(SimRadio* a, SimRadio* b, float loss) { links[{a, b}] = loss; links[{b, a}] = loss; }
inline void Medium::unlink(SimRadio* a, SimRadio* b) { links.erase({a, b}); links.erase({b, a}); }
inline bool Medium::hears(SimRadio* rx, SimRadio* tx) const { return links.count({tx, rx}) > 0; }
inline float Medium::lossOf(SimRadio* a, SimRadio* b) const { auto it = links.find({a, b}); return it == links.end() ? 1.0f : it->second; }

inline void Medium::step(uint32_t now) {
  for (auto& t : txs) {
    if (t.done || t.end > now) continue;
    t.done = true;
    t.from->sending = false;
    for (SimRadio* r : radios) {
      if (r == t.from || !hears(r, t.from)) continue;
      bool lost = rnd() < lossOf(t.from, r);
      // Halbduplex: Empfaenger sendete waehrend dieses Pakets
      for (auto& o : txs) {
        if (&o == &t) continue;
        bool overlap = o.start < t.end && t.start < o.end;
        if (!overlap) continue;
        if (o.from == r) lost = true;                       // Empfaenger sendet selbst
        else if (hears(r, o.from)) lost = true;             // Kollision zweier hoerbarer Sender
      }
      if (!lost) r->rx.push_back(t.bytes);
      if (record_events) events.push_back({t.start, t.end, t.from->name, r->name, (int)t.bytes.size(), t.bytes.empty() ? (uint8_t)0 : t.bytes[0], !lost});
    }
  }
  while (!txs.empty() && txs.front().done && txs.front().end + 120000 < now) txs.pop_front();   // 2 min Verlauf reicht
}

}  // namespace sim
