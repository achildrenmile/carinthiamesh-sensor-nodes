#pragma once
// Minimale Arduino-Attrappe fuer den PC. Zeit und Pins gehoeren der Simulation (sim::).
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <algorithm>
#include <string>

#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define DEC 10
#define HEX 16

#define constrain(x, lo, hi) ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))
typedef bool boolean;
typedef uint8_t byte;
#ifndef PROGMEM
#define PROGMEM
#endif
template <class A, class B> inline auto min(A a, B b) -> decltype(a < b ? a : b) { return a < b ? a : b; }
template <class A, class B> inline auto max(A a, B b) -> decltype(a < b ? b : a) { return a < b ? b : a; }

namespace sim {
inline uint32_t now_ms = 0;                 // gemeinsame Simulationszeit
constexpr int NUM_PINS = 64;
inline uint8_t pin_mode[NUM_PINS];          // wie vom Code gesetzt
inline uint8_t pin_in[NUM_PINS];            // was die "Umwelt" anlegt (HIGH/LOW)
inline bool    pin_in_driven[NUM_PINS];     // false = offen: INPUT_PULLUP liest HIGH
inline uint8_t pin_out[NUM_PINS];           // was der Code ausgibt
inline uint32_t pin_out_changes[NUM_PINS];  // Anzahl Pegelwechsel am Ausgang
}

inline uint32_t millis() { return sim::now_ms; }
inline uint32_t micros() { return sim::now_ms * 1000; }
inline void delay(uint32_t ms) { sim::now_ms += ms; }
inline void pinMode(int pin, int mode) { if (pin >= 0 && pin < sim::NUM_PINS) sim::pin_mode[pin] = (uint8_t)mode; }
inline int digitalRead(int pin) {
  if (pin < 0 || pin >= sim::NUM_PINS) return LOW;
  if (sim::pin_in_driven[pin]) return sim::pin_in[pin];
  return sim::pin_mode[pin] == INPUT_PULLUP ? HIGH : LOW;   // offener Eingang
}
inline void digitalWrite(int pin, int v) {
  if (pin < 0 || pin >= sim::NUM_PINS) return;
  if (sim::pin_out[pin] != v) sim::pin_out_changes[pin]++;
  sim::pin_out[pin] = (uint8_t)v;
}
inline void randomSeed(long s) { srand((unsigned)s); }
inline long random(long lo, long hi) { return lo + rand() % (hi - lo); }

class Print {
public:
  virtual ~Print() = default;
  virtual size_t write(uint8_t) { return 1; }
  virtual size_t write(const uint8_t* b, size_t n) { size_t t = 0; for (size_t i = 0; i < n; i++) t += write(b[i]); return t; }
  size_t write(const char* s) { return s ? write((const uint8_t*)s, strlen(s)) : 0; }
  size_t write(const char* s, size_t n) { return write((const uint8_t*)s, n); }
  size_t print(const char* s) { return write(s); }
  size_t print(char c) { return write((uint8_t)c); }
  size_t print(int v, int base = DEC) { char t[34]; snprintf(t, sizeof t, base == HEX ? "%x" : "%d", v); return write(t); }
  size_t print(unsigned v, int base = DEC) { char t[34]; snprintf(t, sizeof t, base == HEX ? "%x" : "%u", v); return write(t); }
  size_t print(long v, int base = DEC) { return print((int)v, base); }
  size_t print(unsigned long v, int base = DEC) { return print((unsigned)v, base); }
  size_t print(unsigned char v, int base = DEC) { return print((unsigned)v, base); }
  size_t print(double v, int p = 2) { char t[64]; snprintf(t, sizeof t, "%.*f", p, v); return write(t); }
  size_t println() { return write("\r\n"); }
  size_t println(const char* s) { size_t n = write(s); return n + println(); }
  size_t println(int v) { size_t n = print(v); return n + println(); }
  size_t printf(const char* f, ...) __attribute__((format(printf, 2, 3))) {
    char t[512]; va_list a; va_start(a, f); int n = vsnprintf(t, sizeof t, f, a); va_end(a);
    return n > 0 ? write(t) : 0;
  }
  virtual void flush() {}
};

class Stream : public Print {
public:
  virtual int available() { return 0; }
  virtual int read() { return -1; }
  virtual int peek() { return -1; }
  virtual size_t readBytes(char* b, size_t n) { size_t i = 0; while (i < n && available()) b[i++] = (char)read(); return i; }
  virtual size_t readBytes(uint8_t* b, size_t n) { return readBytes((char*)b, n); }
};

class SimSerial : public Stream {
public:
  void begin(unsigned long) {}
  size_t write(uint8_t) override { return 1; }   // Serial-Ausgabe der Firmware wird verworfen
};
inline SimSerial Serial;

inline char* ltoa(long v, char* buf, int base) { snprintf(buf, 34, base == 16 ? "%lx" : "%ld", v); return buf; }
inline char* itoa(int v, char* buf, int base) { return ltoa(v, buf, base); }
#include "FS.h"   // auf der echten Hardware bringt Arduino.h das Dateisystem mit
