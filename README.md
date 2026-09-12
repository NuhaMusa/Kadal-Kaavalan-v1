```markdown
# Kadal Kaavalan
### Sea Guard for Every Fisherman

A fully offline, autonomous maritime boundary alert system for fishermen[cite: 1, 2]. It alerts the crew directly on board before approaching the International Maritime Boundary Line (IMBL) without relying on cellular networks, SIM cards, or recurring subscriptions[cite: 1, 2].

The system uses a distributed multi-controller architecture across two units:
* **Boat Unit:** 1× VEGA ARIES v2 (THEJAS32 RISC-V SoC) + 1× ESP32 (Node 1) + Semtech SX1278 LoRa transceiver + u-blox NEO-6M GPS[cite: 1, 2].
* **Shore Unit:** 1× ESP32 (Node 2) + Semtech SX1278 LoRa receiver + Live Ground Station Dashboard[cite: 1].

Built by **Team ICONIC — Shoreline Labs**, Chennai Institute of Technology (ECE), for **EMBRIX'26 VEGATHON** (BAIT × C-DAC India, Hardware Innovation Track)[cite: 1, 2].

---

## The Problem

Conventional Vessel Monitoring Systems (VMS) report coordinates back to coastal authorities, but fishermen on board receive no direct, real-time feedback[cite: 2]. Because maritime boundaries are invisible and mobile towers lose signal offshore, boats unintentionally drift across the IMBL, leading to vessel seizures and detentions[cite: 1, 2]. 

Kadal Kaavalan closes this gap by calculating boundary proximity directly on board and triggering immediate sensory alerts on deck while forwarding telemetry over long-range LoRa to the shore[cite: 1, 2].

---

## System Architecture


```

[ u-blox NEO-6M GPS ]
│ (UART2)
▼
[ ESP32 Node 1 (Boat) ] ──(SPI)── [ SX1278 LoRa (433 MHz) ]
│                                   │ (RF Packet Broadcast)
(Inter-board                              ▼
UART)                       [ SX1278 LoRa Receiver ]
│                                   │ (SPI)
▼                                   ▼
[ VEGA ARIES v2 (RISC-V) ]        [ ESP32 Node 2 (Shore) ]
├── Multi-color LEDs                      │ (USB Serial / Wi-Fi)
├── Active Alert Buzzer                   ▼
└── Emergency SOS Button       [ Ground Station Dashboard (Python) ]

```

### 1. Boat Unit
* **ESP32 (Node 1 — `KK_ESP_BOAT.ino`):**
  * Reads and parses live NMEA coordinates from the **u-blox NEO-6M GPS** module via hardware UART[cite: 1].
  * Computes distance to the pre-loaded IMBL coordinates and manages geofence state transitions[cite: 1].
  * Broadcasts real-time telemetry packets (Boat ID, latitude, longitude, and safety state) using the **SX1278 LoRa transceiver** over SPI (433 MHz)[cite: 1].
  * Streams geofence status and emergency triggers to the VEGA processor over inter-board UART.
* **VEGA ARIES v2 (`KK_VEGA_BOAT.ino`):**
  * Powered by C-DAC's indigenously developed THEJAS32 RISC-V SoC[cite: 2].
  * Acts as the dedicated Human-Machine Interface (HMI) controller.
  * Controls the visual multi-state LEDs (**Green** for Safe, **Amber** for Warning, **Red** for Danger)[cite: 1].
  * Generates audio warning patterns on the active buzzer[cite: 1].
  * Monitors the physical **SOS button** via hardware interrupts to trigger immediate distress broadcasts[cite: 1, 2].

### 2. Shore Unit (`KK_ESP_SHORE.ino` & `dashboard.py`)
* **ESP32 (Node 2):** Continuously listens for offshore LoRa packets, unpacks vessel telemetry, and streams status logs over USB serial[cite: 1].
* **Dashboard:** A Python monitoring interface (`dashboard.py`) that logs incoming coordinates, displays active vessel zones, and raises audible/visual alarms on Danger or SOS triggers[cite: 1].

---

## Geofence Logic

| Zone | Boundary Distance | Onboard Indication | Shore Action |
| :--- | :--- | :--- | :--- |
| **SAFE** | > 5 Nautical Miles | Green LED ON, Buzzer OFF | Routine heartbeat log[cite: 1] |
| **WARNING** | 1 NM to 5 NM | Amber LED ON, Intermittent Buzzer | Fleet advisory flag[cite: 1] |
| **DANGER** | < 1 Nautical Mile | Red LED ON, Rapid Buzzer | Critical alert broadcast[cite: 1] |
| **SOS** | Triggered anytime | Red LED flashing, Continuous Buzzer | Emergency distress escalation[cite: 1] |

---

## Repository Structure


```

Kadal-Kaavalan-v1/
├── KK_ESP_BOAT.ino       # Firmware for Boat ESP32 (GPS parsing & LoRa TX)
├── KK_VEGA_BOAT.ino      # Firmware for Boat VEGA ARIES v2 (LEDs, Buzzer, SOS)
├── KK_ESP_SHORE.ino      # Firmware for Shore Station ESP32 (LoRa RX & telemetry forwarding)
├── dashboard.py          # Ground station Python dashboard / fleet logger
├── README.md             # Project documentation
└── .gitignore            # Git ignore rules

```

---

## Hardware Pinout Reference

### Boat Unit: ESP32 Node 1
* **u-blox NEO-6M GPS:** `RX` → `GPIO 17 (TX2)`, `TX` → `GPIO 16 (RX2)`, `VCC` → `3.3V`, `GND` → `GND`
* **SX1278 LoRa:** `NSS/CS` → `GPIO 5`, `SCK` → `GPIO 18`, `MOSI` → `GPIO 23`, `MISO` → `GPIO 19`, `RST` → `GPIO 14`, `DIO0` → `GPIO 2`
* **VEGA Inter-chip UART:** `TX` → VEGA `RX`, `RX` → VEGA `TX`, Common `GND`

### Boat Unit: VEGA ARIES v2
* **Status LEDs:** Safe (Green), Warning (Amber), Danger (Red) connected to GPIOs with 220 Ω resistors[cite: 1]
* **Buzzer:** Signal pin connected to PWM GPIO / Transistor driver[cite: 1]
* **SOS Button:** Push button configured with pull-up to Interrupt-capable GPIO[cite: 1]

### Shore Unit: ESP32 Node 2
* **SX1278 LoRa:** `NSS/CS` → `GPIO 5`, `SCK` → `GPIO 18`, `MOSI` → `GPIO 23`, `MISO` → `GPIO 19`, `RST` → `GPIO 14`, `DIO0` → `GPIO 2`

---

## Getting Started

### 1. Flashing Firmware
1. Open the Arduino IDE.
2. Ensure you have board support installed:
   - **ESP32:** Espressif ESP32 board package.
   - **VEGA ARIES v2:** C-DAC VEGA ARIES board package.
3. Install required libraries:
   - `LoRa` (by Sandeep Mistry)
   - `TinyGPS++` (by Mikal Hart)
4. Upload:
   - `KK_ESP_BOAT.ino` to the Boat ESP32.
   - `KK_VEGA_BOAT.ino` to the VEGA ARIES v2 board.
   - `KK_ESP_SHORE.ino` to the Shore ESP32.

### 2. Running the Shore Dashboard
Connect the Shore ESP32 to your workstation via USB and launch the ground station interface:
```bash
python dashboard.py

```

---

## Team

**Team ICONIC — Shoreline Labs**

```

```
