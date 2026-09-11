// ═══════════════════════════════════════════════════════════
// KADAL KAAVALAN — ESP32 Boat Unit
// Geofence state machine + fake GPS
// Sends state to VEGA via UART
// Broadcasts LoRa packet to shore
// Receives LoRa ACK from shore
//
// Wiring:
//   LoRa MOSI  → GPIO23
//   LoRa MISO  → GPIO19
//   LoRa SCK   → GPIO18
//   LoRa NSS   → GPIO5
//   LoRa RST   → GPIO14
//   LoRa DIO0  → GPIO2
//   VEGA RX    → GPIO17 (TX2)
//   VEGA TX    → GPIO16 (RX2)
//   GND        → GND (common with VEGA)
//
// Board  : ESP32 Dev Module
// Team   : Team ICONIC — Shoreline Labs
// ═══════════════════════════════════════════════════════════

#include <SPI.h>
#include <LoRa.h>

// ── LoRa Pins ──────────────────────────────────────────────
#define LORA_NSS   5
#define LORA_RST   14
#define LORA_DIO0  2

// ── LoRa Settings ──────────────────────────────────────────
#define LORA_FREQ  433E6
#define LORA_SF    7
#define LORA_BW    125E3
#define LORA_CR    5
#define LORA_SYNC  0x12

// ── Demo Mode ──────────────────────────────────────────────
#define DEMO_MODE     true
#define STEP_INTERVAL 3000

// ── Geofence Thresholds ────────────────────────────────────
#define WARN_NM   5.0
#define DANGER_NM 1.0

// ── State Machine ──────────────────────────────────────────
enum State { SAFE, WARNING, DANGER, CROSSED };
State currentState = SAFE;

// ── Fake GPS Path ──────────────────────────────────────────
float testPath[][2] = {
  {9.2800, 79.8500},  // SAFE
  {9.2900, 79.9000},  // SAFE
  {9.3100, 79.9500},  // WARNING
  {9.3200, 79.9800},  // DANGER
  {9.3300, 80.0200},  // CROSSED
};
const int PATH_LEN = 5;
int pathIndex = 0;

// ── IMBL Reference ─────────────────────────────────────────
float IMBL_LAT = 9.3250;
float IMBL_LON = 80.0000;

// ── Timing ─────────────────────────────────────────────────
unsigned long lastStepTime = 0;

// ── Node ID ────────────────────────────────────────────────
String nodeID = "KK-001";

// ═══════════════════════════════════════════════════════════
// HAVERSINE
// ═══════════════════════════════════════════════════════════
float toRad(float deg) {
  return deg * 3.14159265 / 180.0;
}

float haversine(float lat1, float lon1,
                float lat2, float lon2) {
  float R    = 3440.065;
  float dLat = toRad(lat2 - lat1);
  float dLon = toRad(lon2 - lon1);
  float a    = sin(dLat/2)*sin(dLat/2) +
               cos(toRad(lat1))*cos(toRad(lat2))*
               sin(dLon/2)*sin(dLon/2);
  return R * 2 * atan2(sqrt(a), sqrt(1-a));
}

// ═══════════════════════════════════════════════════════════
// SEND STATE TO VEGA VIA UART
// ═══════════════════════════════════════════════════════════
void sendToVEGA(String state) {
  Serial2.println(state);
  Serial.print("Sent to VEGA: ");
  Serial.println(state);
}

// ═══════════════════════════════════════════════════════════
// SEND LORA PACKET TO SHORE
// ═══════════════════════════════════════════════════════════
void sendLoRa(float lat, float lon,
              String state, float distNm) {
  LoRa.beginPacket();
  LoRa.print(nodeID);
  LoRa.print(",");
  LoRa.print(state);
  LoRa.print(",");
  LoRa.print(lat, 4);
  LoRa.print(",");
  LoRa.print(lon, 4);
  LoRa.print(",");
  LoRa.print(distNm, 2);
  LoRa.endPacket();

  Serial.print("LoRa TX: ");
  Serial.print(state);
  Serial.print(" | Dist: ");
  Serial.print(distNm, 2);
  Serial.println(" nm");
}

// ═══════════════════════════════════════════════════════════
// SETUP
// ═══════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
  delay(1000);

  Serial.println("========================================");
  Serial.println("  KADAL KAAVALAN — ESP32 Boat Unit");
  Serial.println("  Team ICONIC | Shoreline Labs");
  Serial.println("========================================");

  LoRa.setPins(LORA_NSS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(LORA_FREQ)) {
    Serial.println("LoRa FAILED");
    while (1);
  }

  LoRa.setSpreadingFactor(LORA_SF);
  LoRa.setSignalBandwidth(LORA_BW);
  LoRa.setCodingRate4(LORA_CR);
  LoRa.setSyncWord(LORA_SYNC);

  Serial.println("LoRa OK");
  Serial.println("UART to VEGA OK");
  Serial.println("Starting in 3 seconds...");
  Serial.println("----------------------------------------");
  delay(3000);
}

// ═══════════════════════════════════════════════════════════
// LOOP
// ═══════════════════════════════════════════════════════════
void loop() {
  unsigned long now = millis();

  // ── Step through fake GPS every 3 seconds ────────────────
  if (now - lastStepTime >= STEP_INTERVAL) {
    lastStepTime = now;

    float lat    = testPath[pathIndex][0];
    float lon    = testPath[pathIndex][1];
    float distNm = haversine(lat, lon, IMBL_LAT, IMBL_LON);

    // Determine state
    String stateStr;
    if      (distNm > WARN_NM)   { currentState = SAFE;    stateStr = "SAFE"; }
    else if (distNm > DANGER_NM) { currentState = WARNING;  stateStr = "WARNING"; }
    else                         { currentState = DANGER;   stateStr = "DANGER"; }

    if (lat >= IMBL_LAT && lon >= IMBL_LON) {
      currentState = CROSSED;
      stateStr = "CROSSED";
    }

    Serial.println("----------------------------------------");
    Serial.print("Step ");
    Serial.print(pathIndex + 1);
    Serial.print(" | Lat: ");
    Serial.print(lat, 4);
    Serial.print(" Lon: ");
    Serial.print(lon, 4);
    Serial.print(" | Dist: ");
    Serial.print(distNm, 2);
    Serial.print(" nm | State: ");
    Serial.println(stateStr);

    // Send state to VEGA via UART
    sendToVEGA(stateStr);

    // Send LoRa packet on every state
    sendLoRa(lat, lon, stateStr, distNm);

    pathIndex = (pathIndex + 1) % PATH_LEN;
  }

  // ── Check for incoming LoRa from shore ───────────────────
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String incoming = "";
    while (LoRa.available()) {
      incoming += (char)LoRa.read();
    }
    Serial.print("[SHORE ACK] ");
    Serial.print(incoming);
    Serial.print(" | RSSI: ");
    Serial.print(LoRa.packetRssi());
    Serial.println(" dBm");
  }
}