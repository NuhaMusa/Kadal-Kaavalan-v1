"""
╔══════════════════════════════════════════════════════════╗
  KADAL KAAVALAN — Shore Command Dashboard
  Team ICONIC | Shoreline Labs
  EMbrix'26 VEGATHON — BAIT × C-DAC India
╚══════════════════════════════════════════════════════════╝
"""

import serial
import time
import sys
from datetime import datetime
from rich.console import Console
from rich.panel import Panel
from rich.text import Text
from rich.live import Live
from rich.table import Table
from rich import box
from rich.align import Align

PORT     = "COM5"
BAUDRATE = 115200
DEMO     = "--demo" in sys.argv

console = Console()

data = {
    "node"    : "---",
    "state"   : "WAITING",
    "lat"     : "---",
    "lon"     : "---",
    "dist"    : "---",
    "rssi"    : "---",
    "time"    : "---",
    "log"     : [],
    "sos"     : False,
    "packets" : 0,
}

STATE_CONFIG = {
    "SAFE"    : ("🟢 SAFE",     "bold green",        "green",  "All clear. Vessel within safe zone."),
    "WARNING" : ("🟡 WARNING",  "bold yellow",       "yellow", "Approaching boundary — reduce speed."),
    "DANGER"  : ("🔴 DANGER",   "bold red",          "red",    "Critical zone — turn back immediately!"),
    "CROSSED" : ("🚨 CROSSED",  "bold white on red", "red",    "BOUNDARY CROSSED — Alert coast guard!"),
    "SOS"     : ("🆘 SOS",      "bold white on red", "red",    "MAYDAY — Vessel in distress!"),
    "WAITING" : ("⏳ WAITING",  "bold white",        "white",  "Waiting for signal from vessel..."),
}

def get_state_cfg(state):
    return STATE_CONFIG.get(state, STATE_CONFIG["WAITING"])

def parse_packet(line):
    line = line.strip()
    if line.startswith("PKT:"):
        line = line[4:]
    parts = line.split(",")
    if len(parts) >= 5 and parts[0].startswith("KK"):
        return {
            "node"  : parts[0],
            "state" : parts[1].upper(),
            "lat"   : parts[2],
            "lon"   : parts[3],
            "dist"  : parts[4],
            "rssi"  : parts[5] if len(parts) > 5 else "---",
        }
    return None

def build_dashboard():
    label, style, color, msg = get_state_cfg(data["state"])

    header_text = Text(justify="center")
    header_text.append("⚓  KADAL KAAVALAN", style="bold cyan")
    header_text.append("  —  ", style="dim white")
    header_text.append("SHORE COMMAND DASHBOARD", style="bold white")
    header = Panel(
        Align.center(header_text),
        style="bold cyan",
        box=box.DOUBLE_EDGE,
        padding=(0, 2),
    )

    blink = ""
    if data["state"] in ("CROSSED", "SOS"):
        blink = "  ◉ ALERT ◉" if int(time.time()) % 2 == 0 else "         "

    state_text = Text(justify="center")
    state_text.append(f"\n  {label}{blink}  \n", style=style)
    state_text.append(f"\n  {msg}  \n", style="dim white")

    state_panel = Panel(
        Align.center(state_text),
        title="[bold white]VESSEL STATUS[/bold white]",
        border_style=color,
        box=box.HEAVY,
        padding=(1, 4),
    )

    table = Table(
        show_header=False,
        box=box.SIMPLE,
        padding=(0, 2),
        expand=True,
    )
    table.add_column("Key",   style="dim cyan",   width=16)
    table.add_column("Value", style="bold white",  width=24)

    table.add_row("Node ID",     data["node"])
    table.add_row("Latitude",    f"{data['lat']} °N")
    table.add_row("Longitude",   f"{data['lon']} °E")
    table.add_row("Distance",    f"{data['dist']}")
    table.add_row("Signal",      f"{data['rssi']} dBm")
    table.add_row("Last Update", data["time"])
    table.add_row("Packets Rx",  str(data["packets"]))

    data_panel = Panel(
        table,
        title="[bold cyan]TELEMETRY[/bold cyan]",
        border_style="cyan",
        box=box.ROUNDED,
    )

    legend = Text(justify="center")
    legend.append("  🟢 SAFE  ", style="green")
    legend.append("│", style="dim white")
    legend.append("  🟡 WARNING  ", style="yellow")
    legend.append("│", style="dim white")
    legend.append("  🔴 DANGER  ", style="red")
    legend.append("│", style="dim white")
    legend.append("  🚨 CROSSED / SOS  ", style="bold red")

    legend_panel = Panel(
        Align.center(legend),
        title="[dim]ZONE REFERENCE[/dim]",
        border_style="dim white",
        box=box.SIMPLE_HEAD,
        padding=(0, 1),
    )

    log_text = Text()
    for entry in data["log"][-6:]:
        ts, state, dist = entry
        _, lstyle, _, _ = get_state_cfg(state)
        log_text.append(f"  {ts}  ", style="dim white")
        log_text.append(f"{state:<10}", style=lstyle)
        log_text.append(f"  {dist}\n", style="white")

    log_panel = Panel(
        log_text,
        title="[bold white]EVENT LOG[/bold white]",
        border_style="blue",
        box=box.ROUNDED,
        padding=(0, 1),
    )

    footer_text = Text(justify="center")
    footer_text.append("Team ICONIC", style="bold cyan")
    footer_text.append("  |  ", style="dim white")
    footer_text.append("Shoreline Labs", style="bold white")
    footer_text.append("  |  ", style="dim white")
    footer_text.append("EMbrix'26 VEGATHON", style="bold cyan")
    footer_text.append("  |  ", style="dim white")
    footer_text.append("BAIT × C-DAC India", style="dim white")

    footer = Panel(
        Align.center(footer_text),
        style="dim",
        box=box.SIMPLE,
        padding=(0, 1),
    )

    from rich.console import Group
    return Group(
        header,
        state_panel,
        data_panel,
        legend_panel,
        log_panel,
        footer,
    )

DEMO_SEQUENCE = [
    "KK-001,SAFE,9.2800,79.8500,12.43,-67",
    "KK-001,SAFE,9.2900,79.9000,8.21,-65",
    "KK-001,WARNING,9.3100,79.9500,3.10,-63",
    "KK-001,DANGER,9.3200,79.9800,1.22,-61",
    "KK-001,CROSSED,9.3300,80.0200,0.85,-60",
    "KK-001,SOS,9.3300,80.0200,0.85,-60",
]

def main():
    console.clear()

    if DEMO:
        console.print("[bold yellow]⚡ DEMO MODE — no serial port needed[/bold yellow]")
        ser = None
    else:
        try:
            ser = serial.Serial(PORT, BAUDRATE, timeout=1)
            console.print(f"[bold green]✓ Connected to {PORT} at {BAUDRATE} baud[/bold green]")
            time.sleep(2)
        except Exception as e:
            console.print(f"[bold red]✗ Serial error: {e}[/bold red]")
            console.print("[yellow]Tip: Run with --demo to test without hardware[/yellow]")
            sys.exit(1)

    demo_index = 0
    demo_timer = time.time()

    with Live(build_dashboard(), refresh_per_second=2, screen=True) as live:
        while True:
            try:
                line = None

                if DEMO:
                    if time.time() - demo_timer > 3:
                        line = DEMO_SEQUENCE[demo_index % len(DEMO_SEQUENCE)]
                        demo_index += 1
                        demo_timer = time.time()
                else:
                    if ser.in_waiting:
                        line = ser.readline().decode("utf-8", errors="ignore").strip()

                if line:
                    parsed = parse_packet(line)
                    if parsed:
                        data["node"]    = parsed["node"]
                        data["state"]   = parsed["state"]
                        data["lat"]     = parsed["lat"]
                        data["lon"]     = parsed["lon"]
                        data["dist"]    = parsed["dist"]
                        data["rssi"]    = parsed["rssi"]
                        data["time"]    = datetime.now().strftime("%H:%M:%S")
                        data["packets"] += 1
                        data["sos"]     = parsed["state"] == "SOS"
                        data["log"].append((
                            data["time"],
                            data["state"],
                            data["dist"],
                        ))

                live.update(build_dashboard())
                time.sleep(0.5)

            except KeyboardInterrupt:
                console.print("\n[bold cyan]Dashboard closed.[/bold cyan]")
                break

if __name__ == "__main__":
    main()