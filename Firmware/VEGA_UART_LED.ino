// ═══════════════════════════════════════════════════════════
// KADAL KAAVALAN — VEGA ARIES v2 Boat Unit
// Receives state from ESP32 via UART1
// Drives LEDs based on state
//
// Wiring:
//   LED RED    → GPIO13 (J3 pin 6) via 220Ω
//   LED YELLOW → GPIO14 (J3 pin 5) via 220Ω
//   LED GREEN  → GPIO15 (J3 pin 4) via 220Ω
//   ESP32 TX   → J2 pin 15 (UART1_RX = RX1)
//   ESP32 RX   → J2 pin 13 (UART1_TX = TX1)
//   GND        → GND (common with ESP32)
//
// Board  : VEGA ARIES v2
// Team   : Team ICONIC — Shoreline Labs
// ═══════════════════════════════════════════════════════════

#include <HardwareSerial.h>

HardwareSerial espSerial(1); // UART1 — J2 pin 13/15

#define LED_RED    13
#define LED_YELLOW 14
#define LED_GREEN  15

void setup() {
  Serial.begin(115200);      // USB monitor
  espSerial.begin(9600);     // UART1 — talks to ESP32

  pinMode(LED_RED,    OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN,  OUTPUT);

  digitalWrite(LED_RED,    LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN,  LOW);

  delay(3000);
  Serial.println("========================================");
  Serial.println("  KADAL KAAVALAN — VEGA Boat Unit");
  Serial.println("  Waiting for state from ESP32...");
  Serial.println("========================================");
}

void updateLEDs(String state) {
  digitalWrite(LED_RED,    LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN,  LOW);

  if (state == "SAFE") {
    digitalWrite(LED_GREEN, HIGH);
    Serial.println("[SAFE]    GREEN ON");
  } else if (state == "WARNING") {
    digitalWrite(LED_YELLOW, HIGH);
    Serial.println("[WARNING] YELLOW ON");
  } else if (state == "DANGER") {
    digitalWrite(LED_RED, HIGH);
    Serial.println("[DANGER]  RED ON");
  } else if (state == "CROSSED") {
    digitalWrite(LED_RED, HIGH);
    Serial.println("[CROSSED] RED ON — BOUNDARY CROSSED");
  } else if (state == "SOS") {
    digitalWrite(LED_RED, HIGH);
    Serial.println("[SOS]     RED ON — SOS BROADCAST");
  }
}

String readLine() {
  String line = "";
  while (espSerial.available()) {
    char c = (char)espSerial.read();
    if (c == '\n') break;
    if (c != '\r') line += c;
  }
  return line;
}

void loop() {
  if (espSerial.available()) {
    String msg = readLine();
    msg.trim();
    if (msg.length() > 0) {
      Serial.print("Received from ESP32: ");
      Serial.println(msg);
      updateLEDs(msg);
    }
  }
}