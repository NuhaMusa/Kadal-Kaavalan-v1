// ═══════════════════════════════════════════════════════════
// KADAL KAAVALAN — ESP32 Boat Unit
// With OLED Display
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
//   GPS TX     → GPIO26 (RX1)
//   GPS RX     → GPIO4  (TX1)
//   OLED SDA   → GPIO21
//   OLED SCL   → GPIO22
//
// Board  : ESP32 Dev Module
// Team   : Team ICONIC — Shoreline Labs
// ═══════════════════════════════════════════════════════════

#include <SPI.h>
#include <LoRa.h>
#include <HardwareSerial.h>
#include <TinyGPSPlus.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define DEMO_MODE true

#define LORA_NSS   5
#define LORA_RST   14
#define LORA_DIO0  2
#define LORA_FREQ  433E6
#define LORA_SF    7
#define LORA_BW    125E3
#define LORA_CR    5
#define LORA_SYNC  0x12

#define WARN_NM   5.0
#define DANGER_NM 2.0

#define CENTER_LAT 11.4971
#define CENTER_LON 77.2773

#define STEP_INTERVAL 3000

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

float testPath[][2] = {
  {9.2800, 79.8500},
  {9.2900, 79.9000},
  {9.3100, 79.9500},
  {9.3200, 79.9800},
  {9.3300, 80.0200},
};
const int PATH_LEN = 5;
int pathIndex = 0;

float IMBL_LAT = 9.3250;
float IMBL_LON = 80.0000;

HardwareSerial gpsSerial(1);
TinyGPSPlus gps;

unsigned long lastStepTime = 0;
String nodeID = "KK-001";

// ── Haversine ──────────────────────────────────────────────
float toRad(float deg) { return deg * 3.14159265 / 180.0; }

float haversine(float lat1, float lon1, float lat2, float lon2) {
  float R    = 3440.065;
  float dLat = toRad(lat2 - lat1);
  float dLon = toRad(lon2 - lon1);
  float a = sin(dLat/2)*sin(dLat/2) +
            cos(toRad(lat1))*cos(toRad(lat2))*
            sin(dLon/2)*sin(dLon/2);
  return R * 2 * atan2(sqrt(a), sqrt(1-a));
}

// ── State ──────────────────────────────────────────────────
String getState(float lat, float lon, float distNm) {
  if (lat >= IMBL_LAT && lon >= IMBL_LON) return "CROSSED";
  if (distNm > WARN_NM)   return "SAFE";
  if (distNm > DANGER_NM) return "WARNING";
  return "DANGER";
}

// ── OLED Display ───────────────────────────────────────────
void updateOLED(String state, float lat, float lon, float distNm) {
  display.clearDisplay();

  // Header
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("KADAL KAAVALAN");
  display.drawLine(0, 9, 127, 9, SSD1306_WHITE);

  // Node
  display.setCursor(0, 12);
  display.print("Node: ");
  display.println(nodeID);

  // State — big text
  display.setTextSize(2);
  display.setCursor(0, 24);
  if (state == "SAFE")    display.println("SAFE");
  else if (state == "WARNING")  display.println("WARNING");
  else if (state == "DANGER")   display.println("DANGER");
  else if (state == "CROSSED")  display.println("CROSSED!");
  else if (state == "SOS")      display.println("SOS!!!");

  // Distance
  display.setTextSize(1);
  display.setCursor(0, 48);
  display.print("Dist: ");
  display.print(distNm, 2);
  display.println(" nm");

  // Lat/Lon
  display.setCursor(0, 56);
  display.print(lat, 4);
  display.print(" ");
  display.print(lon, 4);

  display.display();
}

// ── Send to VEGA ───────────────────────────────────────────
void sendToVEGA(String state) {
  Serial2.println(state);
  Serial.print("Sent to VEGA: ");
  Serial.println(state);
}

// ── Send LoRa ──────────────────────────────────────────────
void sendLoRa(float lat, float lon, String state, float distNm) {
  LoRa.beginPacket();
  LoRa.print(nodeID); LoRa.print(",");
  LoRa.print(state);  LoRa.print(",");
  LoRa.print(lat, 4); LoRa.print(",");
  LoRa.print(lon, 4); LoRa.print(",");
  LoRa.print(distNm, 2);
  LoRa.endPacket(false);
  Serial.print("LoRa TX: ");
  Serial.print(state);
  Serial.print(" | Dist: ");
  Serial.print(distNm, 2);
  Serial.println(" nm");
}

// ── Setup ──────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);

#if !DEMO_MODE
  gpsSerial.begin(9600, SERIAL_8N1, 26, 27);
#endif

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED FAILED");
    while (1);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("KADAL KAAVALAN");
  display.println("Shoreline Labs");
  display.println("Booting...");
  display.display();

  delay(1000);
  Serial.println("========================================");
  Serial.println("  KADAL KAAVALAN — ESP32 Boat Unit");
  Serial.println("  Team ICONIC | Shoreline Labs");
  Serial.println("========================================");

  LoRa.setPins(LORA_NSS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(LORA_FREQ)) {
    Serial.println("LoRa FAILED");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("LoRa FAILED!");
    display.display();
    while (1);
  }
  LoRa.setSpreadingFactor(LORA_SF);
  LoRa.setSignalBandwidth(LORA_BW);
  LoRa.setCodingRate4(LORA_CR);
  LoRa.setSyncWord(LORA_SYNC);

  Serial.println("LoRa OK");
  Serial.println("Starting in 3 seconds...");

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("KADAL KAAVALAN");
  display.println("Shoreline Labs");
  display.println("");
  display.println("LoRa OK");
  display.println("Starting...");
  display.display();

  delay(3000);
}

// ── Loop ───────────────────────────────────────────────────
void loop() {
  unsigned long now = millis();
  float lat, lon, distNm;
  String stateStr;

#if DEMO_MODE
  if (now - lastStepTime >= STEP_INTERVAL) {
    lastStepTime = now;

    lat = testPath[pathIndex][0];
    lon = testPath[pathIndex][1];
    distNm = haversine(lat, lon, IMBL_LAT, IMBL_LON);
    stateStr = getState(lat, lon, distNm);

    Serial.println("----------------------------------------");
    Serial.print("Step "); Serial.print(pathIndex + 1);
    Serial.print(" | Lat: "); Serial.print(lat, 4);
    Serial.print(" Lon: "); Serial.print(lon, 4);
    Serial.print(" | Dist: "); Serial.print(distNm, 2);
    Serial.print(" nm | State: "); Serial.println(stateStr);

    updateOLED(stateStr, lat, lon, distNm);
    sendToVEGA(stateStr);
    sendLoRa(lat, lon, stateStr, distNm);

    pathIndex++;
    if (pathIndex >= PATH_LEN) pathIndex = 0;
  }

#else
  unsigned long start = millis();
  while (millis() - start < 1000) {
    while (gpsSerial.available()) gps.encode(gpsSerial.read());
  }

  if (gps.location.isValid() && gps.location.age() < 2000) {
    lat = gps.location.lat();
    lon = gps.location.lng();
    distNm = haversine(lat, lon, CENTER_LAT, CENTER_LON);
    stateStr = getState(lat, lon, distNm);

    Serial.println("----------------------------------------");
    Serial.print("LAT: "); Serial.println(lat, 6);
    Serial.print("LON: "); Serial.println(lon, 6);
    Serial.print("DIST: "); Serial.print(distNm, 2); Serial.println(" nm");
    Serial.print("STATE: "); Serial.println(stateStr);

    updateOLED(stateStr, lat, lon, distNm);
    sendToVEGA(stateStr);
    sendLoRa(lat, lon, stateStr, distNm);
  } else {
    Serial.println("Waiting for GPS fix...");
    sendToVEGA("SAFE");

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("KADAL KAAVALAN");
    display.println("");
    display.println("Waiting for");
    display.println("GPS fix...");
    display.display();
  }
#endif

  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String incoming = "";
    while (LoRa.available()) incoming += (char)LoRa.read();
    Serial.print("[SHORE ACK] "); Serial.println(incoming);
  }
}