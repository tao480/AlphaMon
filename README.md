# The AlphaMon Project Firmware (V4.17)

## System Overview
AlphaMon is a dual-core ESP32-S3 firmware application that bridges renewable energy assets (inverters, batteries, smart meters) to local automation hubs (Home Assistant) via MQTT and long-range telemetry (LoRa MESH).

### Core Software Services
- **Dynamic Timer Service**: Interleaves Modbus RTU/TCP register polling passes based on functional priority tiers.
- **Syntax Checker (SC) & Expression Evaluator (EE)**: Parses custom user automation logic and runs calculations approx 10 times per second to toggle on-board relays and external loads.
- **DDF Engine**: Consumes discrete Device Definition Files (`.417` assets) mapping Modbus registers, functions, scales, and data types down to Home Assistant discovery endpoints.

## Infrastructure Directory Structure
- `/devices`: House asset maps and the Python schema pipeline (`import417.py`, `export417.py`).
- `/src`: Core C++ firmware tracking loops.
- `partitions.csv`: Manages Flash storage boundaries, allowing an expanded 3.5MB application payload block (`app0`/`app1`).
