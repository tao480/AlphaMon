# The AlphaMon Project Firmware (V4.17)

## System Overview
AlphaMon is an edge-based, IoT data collection and forwarding system optimised for enterprise-grade Renewable Energy Demand Management applications.
The AlphaMon's hardware is based on cost effective, dual-core ESP32-S3 processors and a firmware application that bridges renewable energy assets (inverters, batteries, smart meters) to local automation hubs such as Home Assistant, via MQTT and long-range comms using built-in wireless (LoRa) MESH networks for least-cost, last-mile telemetry where WiFi or WAN connectivity isn't readily available.

### Core Software Services
See https://alphamon.net/home/services/ for a more detailed description of the following services:
- **Configuration File Manager**: Synchronises, reads and writes Config File updates, containing the user-defined preferences such as WiFi SSID and credentials, international time zone, LoRa settings, logging and debugging settings, etc.
- **WiFi Client and Access Point Support**: Provides WiFi connectivity for data collection and sharing, including http and htpps protocols, APIs and MQTT connectivity.
- **microSD Card Support**: Allows the end user to store DDF file collections and synchronise Config File settings.
- **LoRa spread spectrum, mesh radio networking**: Solves the "last-mile" cost issue by providing telemetry connectivity in hard-to-reach locations suchas in rural settings, housing estates, large warehousing sites, etc.
- **Modbus RTU and TCP**: Allows AlphaMon devices to connect to one or more Modbus devices such as inverters, smart meters and energy storage systems for data collection and control.  Connectivity is managed by user-defined DDFs (one required per connected device) which defines paramaters such as Slave ID, TCP Port, register names, address and access permissions, Alias Names, etc.
- **Dynamic Timer Service**: Interleaves Modbus RTU/TCP register-level polling passes based on user-defined, functional priority tiers.
- **Syntax Checker (SC) & Expression Evaluator (EE)**: Parses custom user automation logic and runs calculations ~10 times/sec to toggle on-board relays and external loads.
- **DDF Manager**: Consumes discrete Device Definition Files (`.417` assets) mapping Modbus registers, functions, data scaling, and various data types down to MQTT and Home Assistant Discovery endpoints.
- **Event Logging**: The Event Logger service controls how status, warning and error messages are published and displayed across various interfaces including Serial (USB) and UTP messaging.  The reporting levels are user-defined by the Config File settings.

## Infrastructure Directory Structure
- `/assets/devices/DDFs`: Hosts current versions of supported Device Definition File (DDF) assets.
- `/src`: Source code and utilities which may assist in managing AlphaMon devices and networks.
