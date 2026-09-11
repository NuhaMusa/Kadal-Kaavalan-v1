// ═══════════════════════════════════════════════════════════
// KADAL KAAVALAN — ESP32 Shore Unit
// Receives LoRa packets from boat ESP32
// Displays fleet alert on Serial Monitor
//
// Wiring:
//   LoRa MOSI  → GPIO23
//   LoRa MISO  → GPIO19
//   LoRa SCK   → GPIO18
//   LoRa NSS   → GPIO5
//   LoRa RST   → GPIO14
//   LoRa DIO0  → GPIO2
//
// Board  : ESP32 Dev Module
// Team   : Team ICONIC — Shoreline Labs
// ═══════════════════════════════════════════════════════════

#include <SPI.h>
#include <LoRa.h>

#define LORA_NSS   5
#define LORA_RST   14
#define LORA_DIO0  2

#define LORA_FREQ  433E6
#define LORA_SF    7
#define LORA_BW    125E3
#define LORA_CR    5
#define LORA_SYNC  0x12

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("========================================");
  Serial.println("  KADAL KAAVALAN — ESP32 Shore Unit");
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

  Serial.println("LoRa OK — waiting for fleet data");
  Serial.println("----------------------------------------");
}

void loop() {
  int packetSize = LoRa.parsePacket();

  if (packetSize) {
    String packet = "";
    while (LoRa.available()) {
      packet += (char)LoRa.read();
    }

    // Parse: nodeID,state,lat,lon,distNm
    int c1 = packet.indexOf(',');
    int c2 = packet.indexOf(',', c1+1);
    int c3 = packet.indexOf(',', c2+1);
    int c4 = packet.indexOf(',', c3+1);

    String nodeID = packet.substring(0, c1);
    String state  = packet.substring(c1+1, c2);
    String lat    = packet.substring(c2+1, c3);
    String lon    = packet.substring(c3+1, c4);
    String dist   = packet.substring(c4+1);

    Serial.println("========================================");
    if (state == "SOS" || state == "CROSSED") {
      Serial.println("🚨 [SOS / BOUNDARY CROSSED]");
    } else if (state == "DANGER") {
      Serial.println("⚠  [FLEET ALERT — DANGER]");
    } else if (state == "WARNING") {
      Serial.println("⚡ [FLEET ALERT — WARNING]");
    } else {
      Serial.println("✓  [FLEET STATUS — SAFE]");
    }
    Serial.print("Node   : "); Serial.println(nodeID);
    Serial.print("State  : "); Serial.println(state);
    Serial.print("Lat    : "); Serial.println(lat);
    Serial.print("Lon    : "); Serial.println(lon);
    Serial.print("Dist   : "); Serial.print(dist); Serial.println(" nm");
    Serial.print("RSSI   : "); Serial.print(LoRa.packetRssi()); Serial.println(" dBm");
    Serial.println("========================================");

    // Send ACK back to boat
    LoRa.beginPacket();
    LoRa.print("SHORE:ACK:");
    LoRa.print(state);
    LoRa.endPacket();
  }
}