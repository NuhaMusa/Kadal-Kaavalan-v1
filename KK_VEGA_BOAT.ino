// ═══════════════════════════════════════════════════════════
// KADAL KAAVALAN — VEGA ARIES v2 Boat Unit
//
// Wiring:
//   LED RED    → GPIO13 (J3 pin 6) via 220Ω
//   LED YELLOW → GPIO14 (J3 pin 5) via 220Ω
//   LED GREEN  → GPIO15 (J3 pin 4) via 220Ω
//   BUZZER     → GPIO26 (J9 pin 6)
//   SOS BUTTON → GPIO28 (J9 pin 4)
//   ESP32 TX   → J2 pin 15 (UART1_RX)
//   ESP32 RX   → J2 pin 13 (UART1_TX)
//
// Board  : VEGA ARIES v2
// Team   : Team ICONIC — Shoreline Labs
// ═══════════════════════════════════════════════════════════

#include <HardwareSerial.h>

HardwareSerial espSerial(1);

#define LED_RED    13
#define LED_YELLOW 14
#define LED_GREEN  15
#define BUZZER     26
#define SOS_PIN    28

bool sosActive = false;

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

void buzzState(String state) {
  if (state == "WARNING") {
    digitalWrite(BUZZER, HIGH); delay(200);
    digitalWrite(BUZZER, LOW);
  } else if (state == "DANGER") {
    digitalWrite(BUZZER, HIGH); delay(500);
    digitalWrite(BUZZER, LOW);
  } else if (state == "CROSSED") {
    digitalWrite(BUZZER, HIGH); delay(1000);
    digitalWrite(BUZZER, LOW);
  } else if (state == "SOS") {
    for (int i = 0; i < 3; i++) {
      digitalWrite(BUZZER, HIGH); delay(800);
      digitalWrite(BUZZER, LOW);  delay(200);
    }
  } else {
    digitalWrite(BUZZER, LOW);
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

void setup() {
  Serial.begin(115200);
  espSerial.begin(9600);

  pinMode(LED_RED,    OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN,  OUTPUT);
  pinMode(BUZZER,     OUTPUT);
  pinMode(SOS_PIN,    INPUT);

  digitalWrite(LED_RED,    LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN,  LOW);
  digitalWrite(BUZZER,     LOW);

  delay(3000);
  Serial.println("========================================");
  Serial.println("  KADAL KAAVALAN — VEGA Boat Unit");
  Serial.println("  Waiting for state from ESP32...");
  Serial.println("========================================");
}

void loop() {
  if (digitalRead(SOS_PIN) == HIGH) {
    delay(50);
    if (digitalRead(SOS_PIN) == HIGH && !sosActive) {
      sosActive = true;
      Serial.println("SOS TRIGGERED!");
      espSerial.println("SOS");
      updateLEDs("SOS");
      buzzState("SOS");
    }
  } else {
    sosActive = false;
  }

  if (!sosActive && espSerial.available()) {
    String msg = readLine();
    msg.trim();
    if (msg.length() > 0) {
      Serial.print("Received: ");
      Serial.println(msg);
      updateLEDs(msg);
      buzzState(msg);
    }
  }
}