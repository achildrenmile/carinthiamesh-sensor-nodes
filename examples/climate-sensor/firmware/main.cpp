#include "SensorMesh.h"

#ifdef DISPLAY_CLASS
  #include "UITask.h"
  static UITask ui_task(display);
#endif

// ===== Klima-Schwellen =====
#define ENV_CHANNEL     2          // erster Umweltsensor (BME280) liegt auf LPP-Kanal 2
#define FROST_ON_C      2.0f       // Frost-Alert unter 2 °C
#define FROST_OFF_C     4.0f       // zuruecksetzen erst ueber 4 °C (Hysterese)
#define HUMID_ON_PCT    70.0f      // Schimmel-Alert ueber 70 % rF
#define HUMID_OFF_PCT   65.0f

class MyMesh : public SensorMesh {
public:
  MyMesh(mesh::MainBoard& board, mesh::Radio& radio, mesh::MillisecondClock& ms, mesh::RNG& rng, mesh::RTCClock& rtc, mesh::MeshTables& tables)
     : SensorMesh(board, radio, ms, rng, rtc, tables),
       battery_data(12*24, 5*60),   // 24 hours worth of data, every 5 minutes
       temp_data(12*24, 5*60),
       hum_data(12*24, 5*60)
  {
  }

protected:
  /* ========================== custom logic here ========================== */
  Trigger low_batt, critical_batt, frost_alert, humid_alert;
  TimeSeriesData  battery_data, temp_data, hum_data;
  bool frost = false, humid = false;   // Zustand mit Hysterese

  void onSensorDataRead() override {
    float batt_voltage = getVoltage(TELEM_CHANNEL_SELF);

    battery_data.recordData(getRTCClock(), batt_voltage);   // record battery
    bool has_batt = batt_voltage > 2.5f;   // ohne Akku (nur USB) keine Akku-Alarme
    alertIf(has_batt && batt_voltage < 3.4f, critical_batt, HIGH_PRI_ALERT, "Battery is critical!");
    alertIf(has_batt && batt_voltage < 3.6f, low_batt, LOW_PRI_ALERT, "Battery is low");

    float rh = getRelativeHumidity(ENV_CHANNEL);
    if (rh <= 0.0f) return;                 // kein BME280 gefunden: nichts auswerten
    float t = getTemperature(ENV_CHANNEL);

    temp_data.recordData(getRTCClock(), t);
    hum_data.recordData(getRTCClock(), rh);

    if (t < FROST_ON_C) frost = true; else if (t > FROST_OFF_C) frost = false;
    if (rh > HUMID_ON_PCT) humid = true; else if (rh < HUMID_OFF_PCT) humid = false;
    alertIf(frost, frost_alert, LOW_PRI_ALERT, "Frostgefahr");
    alertIf(humid, humid_alert, LOW_PRI_ALERT, "Luftfeuchte hoch");
  }

  int querySeriesData(uint32_t start_secs_ago, uint32_t end_secs_ago, MinMaxAvg dest[], int max_num) override {
    int n = 0;
    if (n < max_num) battery_data.calcMinMaxAvg(getRTCClock(), start_secs_ago, end_secs_ago, &dest[n++], TELEM_CHANNEL_SELF, LPP_VOLTAGE);
    if (n < max_num) temp_data.calcMinMaxAvg(getRTCClock(), start_secs_ago, end_secs_ago, &dest[n++], ENV_CHANNEL, LPP_TEMPERATURE);
    if (n < max_num) hum_data.calcMinMaxAvg(getRTCClock(), start_secs_ago, end_secs_ago, &dest[n++], ENV_CHANNEL, LPP_RELATIVE_HUMIDITY);
    return n;
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
  sensors.loop();
#ifdef DISPLAY_CLASS
  ui_task.loop();
#endif
  rtc_clock.tick();
#ifdef HAS_EXTERNAL_WATCHDOG
  external_watchdog.loop();
#endif
}
