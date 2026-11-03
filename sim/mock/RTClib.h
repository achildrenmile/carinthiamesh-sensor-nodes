#pragma once
#include <stdint.h>
// Minimales DateTime (UTC), reicht fuer die Uhr-Befehle der CLI.
class DateTime {
  int y_, mo_, d_, h_, mi_, s_;
public:
  DateTime(uint32_t t) {
    s_ = t % 60; mi_ = (t / 60) % 60; h_ = (t / 3600) % 24;
    int days = t / 86400, y = 1970;
    for (;;) { int yl = ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0) ? 366 : 365; if (days < yl) break; days -= yl; y++; }
    static const int ml[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    bool leap = ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0);
    int m = 0; while (days >= ml[m] + (m == 1 && leap)) { days -= ml[m] + (m == 1 && leap); m++; }
    y_ = y; mo_ = m + 1; d_ = days + 1;
  }
  int year() const { return y_; } int month() const { return mo_; } int day() const { return d_; }
  int hour() const { return h_; } int minute() const { return mi_; } int second() const { return s_; }
};
