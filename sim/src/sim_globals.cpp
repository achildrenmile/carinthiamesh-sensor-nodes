#include <target.h>
sim::Medium g_medium;
SimBoard board;
sim::SimRadio radio_driver(g_medium, "sensor");
SimSensors sensors;
VolatileRTCClock rtc_clock;
