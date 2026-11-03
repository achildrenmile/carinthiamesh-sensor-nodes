#include "SensorMesh.h"

#ifdef DISPLAY_CLASS
  #include "UITask.h"
  static UITask ui_task(display);
#endif

// ===== Taster, LED, Buzzer =====
// Pins per Build-Flag aenderbar, z. B. -D BTN1_PIN=17 (Default: Heltec V3)
#ifndef BTN1_PIN
  #define BTN1_PIN     4
#endif
#ifndef BTN2_PIN
  #define BTN2_PIN     5
#endif
#ifndef BTN3_PIN
  #define BTN3_PIN     6
#endif
#ifndef BTN4_PIN
  #define BTN4_PIN     7
#endif
#ifndef LED_PIN
  #define LED_PIN      41
#endif
#ifndef BUZZER_PIN
  #define BUZZER_PIN   42
#endif
static const uint8_t BTN_PINS[4] = { BTN1_PIN, BTN2_PIN, BTN3_PIN, BTN4_PIN };   // Taster gegen GND
#define DEBOUNCE_MS    50          // Entprellen
#define COOLDOWN_MS    30000UL     // max. 1 Meldung pro Taster alle 30 s
#define MAX_PER_HOUR   20          // Notbremse: max. 20 Meldungen pro Stunde
#define FEEDBACK_MS    120         // LED/Buzzer-Dauer bei Druck

class MyMesh : public SensorMesh {
public:
  MyMesh(mesh::MainBoard& board, mesh::Radio& radio, mesh::MillisecondClock& ms, mesh::RNG& rng, mesh::RTCClock& rtc, mesh::MeshTables& tables)
     : SensorMesh(board, radio, ms, rng, rtc, tables), 
       battery_data(12*24, 5*60)    // 24 hours worth of battery data, every 5 minutes
  {
  }

  void beginInputs() {
    for (int i = 0; i < 4; i++) pinMode(BTN_PINS[i], INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
  }

  void pollInputs() {
    unsigned long now = millis();
    if (feedback_until && now > feedback_until) {   // Rueckmeldung beenden
      digitalWrite(LED_PIN, LOW);
      digitalWrite(BUZZER_PIN, LOW);
      feedback_until = 0;
    }
    for (int i = 0; i < 4; i++) {
      bool pressed = (digitalRead(BTN_PINS[i]) == LOW);
      if (pressed != raw[i]) { raw[i] = pressed; changed_at[i] = now; }
      if (now - changed_at[i] < DEBOUNCE_MS || pressed == stable[i]) continue;
      stable[i] = pressed;
      if (!pressed) continue;                 // nur Druecken melden, Loslassen nicht

      digitalWrite(LED_PIN, HIGH);            // lokale Rueckmeldung, kostet keine Airtime
      digitalWrite(BUZZER_PIN, HIGH);
      feedback_until = now + FEEDBACK_MS;

      if (last_sent[i] && now - last_sent[i] < COOLDOWN_MS) continue;  // Cooldown
      if (!budgetOk(now)) continue;                                     // Stundenbudget
      last_sent[i] = now;

      char text[32];
      snprintf(text, sizeof(text), "Taster %d gedrueckt", i + 1);
      alertIf(false, btn_alert[i], LOW_PRI_ALERT, "");   // alten Alert zuruecksetzen
      alertIf(true,  btn_alert[i], LOW_PRI_ALERT, text); // neuen Alert einreihen
    }
  }

protected:
  /* ========================== custom logic here ========================== */
  Trigger btn_alert[4];
  bool raw[4] = {}, stable[4] = {};
  unsigned long changed_at[4] = {}, last_sent[4] = {};
  unsigned long hour_start = 0, feedback_until = 0;
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
    // Aktor-Befehle, aus der Companion-App ueber die Admin-Konsole des Sensors
    if (strcmp(command, "led on") == 0)  { digitalWrite(LED_PIN, HIGH); strcpy(reply, "LED an");  return true; }
    if (strcmp(command, "led off") == 0) { digitalWrite(LED_PIN, LOW);  strcpy(reply, "LED aus"); return true; }
    if (strcmp(command, "beep") == 0) {
      digitalWrite(BUZZER_PIN, HIGH);
      feedback_until = millis() + 500;   // pollInputs() schaltet wieder ab
      strcpy(reply, "Beep");
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
