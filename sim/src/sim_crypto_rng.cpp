// Ersatz fuer Crypto/RNG.cpp (ESP32-NVS): deterministische Zufallszahlen, damit Tests reproduzierbar sind.
#include <stdint.h>
#include <stddef.h>
#include <RNG.h>
RNGClass::RNGClass() {}
RNGClass::~RNGClass() {}
void RNGClass::rand(uint8_t* data, size_t len) {
  static uint64_t s = 0x9E3779B97F4A7C15ull;
  for (size_t i = 0; i < len; i++) { s ^= s << 13; s ^= s >> 7; s ^= s << 17; data[i] = (uint8_t)(s >> 24); }
}
RNGClass RNG;
