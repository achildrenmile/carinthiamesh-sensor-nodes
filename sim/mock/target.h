#pragma once
// Stellt die Board-Globale bereit, die SensorMesh.cpp aus target.h erwartet.
#include <Arduino.h>
#include <Mesh.h>
#include <CayenneLPP.h>
#include <helpers/SensorManager.h>
#include <helpers/ArduinoHelpers.h>
#include "sim_radio.h"

class SimBoard : public mesh::MainBoard {
public:
  uint16_t batt_mv = 4000;
  uint16_t getBattMilliVolts() override { return batt_mv; }
  const char* getManufacturerName() const override { return "Sim"; }
  void reboot() override {}
  template <class T> void attachDynamicPrefs(T) {}
  uint8_t getStartupReason() const override { return BD_STARTUP_NORMAL; }
};

// Umweltsensor-Attrappe: liefert wie ein BME280 Temperatur, Feuchte, Druck auf Kanal 2.
class SimSensors : public SensorManager {
public:
  bool bme280 = false;
  float temp_c = 20.0f, humidity = 50.0f, pressure_hpa = 1013.0f;
  bool begin() override { return true; }
  bool querySensors(uint8_t perm, CayenneLPP& lpp) override {
    if (bme280 && (perm & TELEM_PERM_ENVIRONMENT)) {
      lpp.addTemperature(2, temp_c);
      lpp.addRelativeHumidity(2, humidity);
      lpp.addBarometricPressure(2, pressure_hpa);
    }
    return true;
  }
};

extern SimBoard board;
extern sim::SimRadio radio_driver;
extern SimSensors sensors;
extern VolatileRTCClock rtc_clock;
