# Kadal Kaavalan
### Sea Guard for Every Fisherman

A fully offline maritime boundary alert device for fishermen. It warns the fisherman directly on his boat before he approaches the International Maritime Boundary Line (IMBL) — no SIM, no cellular signal, no subscription required.

**This hackathon prototype runs on 2×18650 lithium batteries.** Solar charging is on the roadmap (see Future Plans) but is not part of this build.

Built by **Team ICONIC — Shoreline Labs**, Chennai Institute of Technology (ECE), for **EMbrix'26 VEGATHON** (BAIT × C-DAC India, Hardware Innovation Track).

## The problem

Existing Vessel Monitoring Systems (VMS) report a fisherman's location to the coast guard — but the fisherman himself gets no warning. The gap between what the coast guard knows and what the fisherman knows is what leads to accidental boundary crossings and arrests. Kadal Kaavalan closes that gap by warning the fisherman directly, on-device, in real time.

## How it works

- **Boat unit** (VEGA ARIES v2, THEJAS32 RISC-V): runs a geofence state machine (SAFE / WARNING / DANGER / CROSSED) using GPS + Haversine distance to the boundary. Drives LEDs, an OLED display, audio alerts, and a BMP280-based storm warning. Sends LoRa packets to shore on DANGER/CROSSED/SOS. A physical **SOS button** triggers an instant, repeating LoRa broadcast with GPS coordinates; an MPU9250 also auto-triggers an SOS if the boat tilts past 60° for more than 2 seconds (man-overboard detection).
- **Shore unit** (ESP32): receives LoRa packets, prints fleet alerts, and serves a live web dashboard over its own WiFi access point — the dashboard turns red on any SOS or man-overboard alert.
- Runs entirely offline — the boundary alert to the fisherman never depends on LoRa, WiFi, or any network being up.

## Repo structure

```
kadal-kaavalan/
├── firmware/
│   ├── vega-boat-unit/       Boat unit firmware, one folder per build layer
│   └── esp32-shore-unit/     Shore unit firmware, one folder per build layer
└── docs/                     Wiring diagrams, workflow playbook, explainer doc
```

## Build layers

The firmware is built in four independently demo-able layers — if time runs out, whichever layer is complete is still a working demo.

| Layer | What's added |
|---|---|
| 1 — MVP | Geofence state machine + LEDs |
| 2 — Core | LoRa link (boat ↔ shore) + SOS button |
| 3 — Full | OLED display + BMP280 storm warning + audio alerts |
| 4 — Polish | MPU9250 man-overboard detection + real GPS + web dashboard |

## Flashing the firmware

1. Open the relevant `.ino` file for your layer in Arduino IDE.
2. Select the correct board (VEGA ARIES for the boat unit, an ESP32 dev board for the shore unit) and COM port under **Tools**.
3. Install any missing libraries via **Sketch → Include Library → Manage Libraries** (LoRa by Sandeep Mistry, Adafruit SSD1306/GFX/BMP280/MPU6050, TinyGPS++).
4. Verify/Compile, then Upload.
5. Open the Serial Monitor at **115200 baud** to watch state/packet output.

## Future plans

- Solar charging enclosure (today's build is battery-only)
- Satellite fallback for fleet-to-coast-guard range beyond LoRa mesh
- Partnership with fishermen associations for field trials

## Team

Team ICONIC — Shoreline Labs, Chennai Institute of Technology
