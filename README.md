# Kadal Kaavalan
### Sea Guard for Every Fisherman

A fully offline maritime boundary alert device for fishermen. It warns the fisherman directly on his boat before he approaches the International Maritime Boundary Line (IMBL) — no SIM, no cellular signal, no subscription required.

**This hackathon prototype runs on 2×18650 lithium batteries.**Solar charging is on the roadmap (see Future Plans) but is not part of this build.

Built by **Team ICONIC — Shoreline Labs**, Chennai Institute of Technology (ECE), for **EMBRIX'26 VEGATHON** (BAIT × C-DAC India, Hardware Innovation Track).

## The problem

Existing Vessel Monitoring Systems (VMS) report a fisherman's location to the coast guard — but the fisherman himself gets no warning. The gap between what the coast guard knows and what the fisherman knows is what leads to accidental boundary crossings and arrests. Kadal Kaavalan closes that gap by warning the fisherman directly, on-device, in real time.

## System architecture & workflow

The system uses a distributed three-node architecture across the boat and shore:


```

[ GPS Module (u-blox NEO-6M) ]
│
▼
[ VEGA ARIES v2 (Boat Core) ] ── (Peripherals: LEDs, Buzzer, BMP280, MPU9250, SOS Button)
│
UART2 (State String)
▼
[ ESP32 #1 (Boat Transceiver) ]
│
LoRa Packet (SX1278)
▼
[ ESP32 #2 (Shore Receiver) ] ──▶ Serial Monitor Fleet Alert Logging

```

- **Boat Unit (Core Processor - VEGA ARIES v2, THEJAS32 RISC-V):**
  - Reads GPS data (u-blox NEO-6M).
  - Runs the local geofence engine to evaluate zone status: `SAFE`, `WARNING`, `DANGER`, or `CROSSED`.
  - Drives local indicators (LEDs + buzzer), environmental storm monitoring (BMP280), tilt/man-overboard detection (MPU9250), and monitors the hardware SOS button.
  - Transmits serialized status packets to the onboard transmitter over **UART2**:
    ```
    STATE:DANGER,LAT:9.3200,LON:79.9800\n
    ```
- **Boat Unit (Transmitter Node - ESP32 #1):**
  - Interfaces directly with the VEGA processor via UART.
  - Packages incoming state, coordinate, and SOS triggers into LoRa packets.
  - Broadcasts packets long-range over Semtech SX1278 LoRa to the coast.
- **Shore Station (Receiver Node - ESP32 #2):**
  - Continuously listens for incoming LoRa transmissions from the fleet.
  - Decodes packets and outputs real-time fleet boundary and emergency alerts to the Serial Monitor.

## Repo structure


```

kadal-kaavalan/
├── firmware/
│   ├── KK_VEGA/              VEGA ARIES v2 firmware (GPS, geofence, sensors, UART output)
│   ├── KK_ESP32_Boat/        ESP32 #1 firmware (UART receiver → LoRa broadcaster)
│   └── KK_ESP32_Shore/       ESP32 #2 firmware (LoRa receiver → Serial Monitor fleet alerts)
└── docs/                     Wiring diagrams, workflow playbook, explainer doc

```

## Geofence thresholds

| Zone | Distance to IMBL | Local Warning Behavior | LoRa Broadcast |
|---|---|---|---|
| **SAFE** | > 5 NM | Normal operation / Green LED | Routine heartbeat |
| **WARNING** | 5 NM – 1 NM | Yellow LED indicator | Status update |
| **DANGER** | 0 NM – 1 NM | Red LED + Buzzer audible alert | High-priority alert |
| **CROSSED** | < 0 NM (Boundary Breached) | Continuous Buzzer + Rapid Red Flash | Critical breach alarm |
| **SOS** | Triggered via Button / MPU9250 | Immediate alarm state | Instant emergency alert |

## Flashing the firmware

1. Open Arduino IDE.
2. Flash **`KK_VEGA.ino`** to the **VEGA ARIES v2** board.
3. Flash **`KK_ESP32_Boat.ino`** to the boat's **ESP32 #1** connected to the SX1278 LoRa module.
4. Flash **`KK_ESP32_Shore.ino`** to the shore's **ESP32 #2** connected to the receiving LoRa module.
5. Install necessary libraries via **Sketch → Include Library → Manage Libraries** (`LoRa` by Sandeep Mistry, `TinyGPS++`, `Adafruit BMP280`, `Adafruit MPU6050` / MPU9250 drivers).
6. Open the Serial Monitor for ESP32 #2 at **115200 baud** to monitor live incoming fleet transmissions.

## Future plans

- Solar charging enclosure integration (prototype is currently battery-powered)
- Dedicated shore dashboard interface and multi-hop LoRa mesh networking
- Satellite fallback for extended offshore zones beyond LoRa range
- Field trials with local coastal fishing communities

## Team

Team ICONIC — Shoreline Labs, Chennai Institute of Technology
