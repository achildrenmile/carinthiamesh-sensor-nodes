#include "SensorMesh.h"

#ifdef DISPLAY_CLASS
  #include "UITask.h"
  static UITask ui_task(display);
#endif

// ===== Wassermelder =====
#define LEAK_PIN        5          // Digital-Ausgang (DO) des Sensor-Moduls
#define LEAK_WET_LEVEL  LOW        // Pegel bei Nässe; je nach Modul LOW oder HIGH, siehe Anleitung
#define DEBOUNCE_MS     2000       // Wasser muss 2 s anliegen
#define REARM_MS        600000UL   // nach einer Meldung 10 min Ruhe
#define MAX_PER_HOUR    6          // Notbremse: max. 6 Meldungen pro Stunde

class MyMesh : public SensorMesh {
public:
  MyMesh(mesh::MainBoard& board, mesh::Radio& radio, mesh::MillisecondClock& ms, mesh::RNG& rng, mesh::RTCClock& rtc, mesh::MeshTables& tables)
     : SensorMesh(board, radio, ms, rng, rtc, tables),
       battery_data(12*24, 5*60)    // 24 hours worth of battery data, every 5 minutes
  {
  }

  void beginInputs() {
    pinMode(LEAK_PIN, INPUT);       // Modul treibt den Pegel selbst
  }

  void pollInputs() {
    unsigned long now = millis();
    bool wet = (digitalRead(LEAK_PIN) == LEAK_WET_LEVEL);
    if (wet != raw) { raw = wet; changed_at = now; }
    if (now - changed_at < DEBOUNCE_MS || wet == stable) return;
    stable = wet;

    if (!wet) {                                   // wieder trocken: Alert zuruecksetzen
      alertIf(false, leak_alert, HIGH_PRI_ALERT, "");
      return;
    }
    if (last_sent && now - last_sent < REARM_MS) return;   // Ruhezeit
    if (!budgetOk(now)) return;                            // Stundenbudget
    last_sent = now;
    // HIGH_PRI wiederholt, bis ein Companion den Empfang bestaetigt
    alertIf(true, leak_alert, HIGH_PRI_ALERT, "Wasser erkannt!");
  }

protected:
  /* ========================== custom logic here ========================== */
  Trigger leak_alert;
  bool raw = false, stable = false;
  unsigned long changed_at = 0, last_sent = 0, hour_start = 0;
  uint8_t sent_this_hour = 0;

  bool budgetOk(unsigned long now) {
    if (now - hour_start > 3600000UL) { hour_start = now; sent_this_hour = 0; }
    if (sent_this_hour >= MAX_PER_HOUR) return false;
    sent_this_hour++;
    return true;
  }
  Trigger low_batt, critical_batt;
  TimeSeriesData  battery_data;

  void onSensorDataRead() override {
    float batt_voltage = getVoltage(TELEM_CHANNEL_SELF);

    battery_data.recordData(getRTCClock(), batt_voltage);   // record battery
    bool has_batt = batt_voltage > 2.5f;   // ohne Akku (nur USB) keine Akku-Alarme
    alertIf(has_batt && batt_voltage < 3.4f, critical_batt, HIGH_PRI_ALERT, "Battery is critical!");
    alertIf(has_batt && batt_voltage < 3.6f, low_batt, LOW_PRI_ALERT, "Battery is low");
  }

  int querySeriesData(uint32_t start_secs_ago, uint32_t end_secs_ago, MinMaxAvg dest[], int max_num) override {
    battery_data.calcMinMaxAvg(getRTCClock(), start_secs_ago, end_secs_ago, &dest[0], TELEM_CHANNEL_SELF, LPP_VOLTAGE);
    return 1;
  }

  bool handleCustomCommand(uint32_t sender_timestamp, char* command, char* reply) override {
    if (strcmp(command, "status") == 0) {
      strcpy(reply, stable ? "nass" : "trocken");
      return true;
    }
    return false;  // not handled
  }
  /* ======================================================================= */
};

StdRNG fast_rng;
SimpleMeshTables tables;

MyMesh the_mesh(board, radio_driver, *new ArduinoMillis(), fast_rng, rtc_clock, tables);

void halt() {
  while (1) ;
}

static char command[160];

void setup() {
  Serial.begin(115200);
  delay(1000);

  board.begin();

#ifdef HAS_EXTERNAL_WATCHDOG
  external_watchdog.begin();
#endif

#ifdef DISPLAY_CLASS
  if (display.begin()) {
    display.startFrame();
    display.print("Please wait...");
    display.endFrame();
  }
#endif

  if (!radio_init()) { halt(); }

  fast_rng.begin(radio_driver.getRngSeed());

  FILESYSTEM* fs;
#if defined(NRF52_PLATFORM) || defined(STM32_PLATFORM)
  InternalFS.begin();
  fs = &InternalFS;
  IdentityStore store(InternalFS, "");
#elif defined(ESP32)
  SPIFFS.begin(true);
  fs = &SPIFFS;
  IdentityStore store(SPIFFS, "/identity");
#elif defined(RP2040_PLATFORM)
  LittleFS.begin();
  fs = &LittleFS;
  IdentityStore store(LittleFS, "/identity");
  store.begin();
#else
  #error "need to define filesystem"
#endif
  if (!store.load("_main", the_mesh.self_id)) {
    MESH_DEBUG_PRINTLN("Generating new keypair");
    the_mesh.self_id = radio_new_identity();   // create new random identity
    int count = 0;
    while (count < 10 && (the_mesh.self_id.pub_key[0] == 0x00 || the_mesh.self_id.pub_key[0] == 0xFF)) {  // reserved id hashes
      the_mesh.self_id = radio_new_identity(); count++;
    }
    store.save("_main", the_mesh.self_id);
  }

  Serial.print("Sensor ID: ");
  mesh::Utils::printHex(Serial, the_mesh.self_id.pub_key, PUB_KEY_SIZE); Serial.println();

  command[0] = 0;

  sensors.begin();

  the_mesh.begin(fs);
  the_mesh.beginInputs();

#ifdef DISPLAY_CLASS
  ui_task.begin(the_mesh.getNodePrefs(), FIRMWARE_BUILD_DATE, FIRMWARE_VERSION);
#endif

  // send out initial zero hop Advertisement to the mesh
#if ENABLE_ADVERT_ON_BOOT == 1
  the_mesh.sendSelfAdvertisement(16000, false);
#endif
}

void loop() {
  int len = strlen(command);
  // `command` must stay NUL-terminated within its bounds. If it ever isn't,
  // strlen() above can return >= sizeof(command) and the loop below would then
  // index past the buffer, so clamp defensively.
  if (len >= (int)sizeof(command)) {
    command[0] = 0;
    len = 0;
  }
  while (Serial.available() && len < sizeof(command)-1) {
    char c = Serial.read();
    if (c != '\n') {
      command[len++] = c;
      command[len] = 0;
    }
    Serial.print(c);
  }
  if (len == sizeof(command)-1) {  // buffer full: treat as a completed line
    command[sizeof(command)-2] = '\r';  // place end-of-line marker inside the buffer
    command[sizeof(command)-1] = 0;     // keep the buffer NUL-terminated
  }

  if (len > 0 && command[len - 1] == '\r') {  // received complete line
    command[len - 1] = 0;  // replace newline with C string null terminator
    char reply[160];
    the_mesh.handleCommand(NULL, 0, command, reply);  // NOTE: there is no sender_timestamp via serial!
    if (reply[0]) {
      Serial.print("  -> "); Serial.println(reply);
    }

    command[0] = 0;  // reset command buffer
  }

  board.loop();   // let the board feed its watchdog, run periodic housekeeping

  the_mesh.loop();
  the_mesh.pollInputs();
  sensors.loop();
#ifdef DISPLAY_CLASS
  ui_task.loop();
#endif
  rtc_clock.tick();
#ifdef HAS_EXTERNAL_WATCHDOG
  external_watchdog.loop();
#endif
}
