#pragma once
// Der Sensor unter Test. Die Klasse MyMesh stammt unveraendert aus der Beispiel-main.cpp
// (siehe gen_app.sh), hier wird sie nur mit Simulations-Globals verbunden.
#include "sim_world.h"
#include <target.h>
#include "app.h"       // erzeugt aus examples/<name>/firmware/main.cpp

namespace sim {

template <class T> auto call_begin_inputs(T& m, int) -> decltype(m.beginInputs(), void()) { m.beginInputs(); }
template <class T> void call_begin_inputs(T&, long) {}
template <class T> auto call_poll_inputs(T& m, int) -> decltype(m.pollInputs(), void()) { m.pollInputs(); }
template <class T> void call_poll_inputs(T&, long) {}

struct SensorNode {
  SimMillis ms; SimRNG rng; SimFS fs; SimpleMeshTables tables;
  MyMesh mesh;
  explicit SensorNode(uint64_t seed = 7)
    : rng(seed), mesh(board, radio_driver, ms, rng, rtc_clock, tables) {
    mesh.self_id = mesh::LocalIdentity(&rng);
    sensors.begin();
    mesh.begin(&fs);
    call_begin_inputs(mesh, 0);
  }
  void tick() {
    mesh.loop();
    call_poll_inputs(mesh, 0);
    rtc_clock.tick();
    sensors.loop();
  }
};

}  // namespace sim
