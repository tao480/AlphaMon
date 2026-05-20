# The AlphaMon Project Firmware (V4.17)

## System Overview
AlphaMon is an IoT, edge-based data collection and forwarding system optimised for enterprise-grade Energy Demand Management and similar applications.
The AlphaMon's hardware is based on cost effective, dual-core ESP32-S3 processors and firmware that bridges grid and renewable energy assets (inverters, batteries, smart meters) to local automation hubs such as Home Assistant, via USB Serial ports, WiFi, MQTT message brokers and long-range (LoRa) wireless MESH networks for least-cost, last-mile telemetry where WiFi or WAN connectivity isn't readily available.

Solargy has developed a demonstration 95mm x 95mm PCB which hosts a Heltec ESP32 LoRa V3 sub-board which runs The AlphaMon Platform firmware. It has been designed to fit into a standard 108mm (L) x 108mm (W) x 75mm (H) IP67-rated electrical enclosure for outdoor mounting. We've also designed proof-of-concept, 3D printed enclosures for indoor mounting.
The current PCB design includes the following features:
- **An independent 12V power supply**: Allows the PCB to be powered from a standard wall socket plug-pack power supply. 
- **An RJ-45 Modbus socket**: Allows quick and easy connections to many inverters and energy storage systems.
- **An RS-485 module**: Which implements the Modbus communication and electrical standards.
- **A microSD Card sub-board**: The optional microSD card can simplify loading and testing configuration and device definitions (DDFs).
- **A voltage controlled relay driver**: Which allows the AlphaMon to switch external electrical loads via a Solid State Relay (SSR).

###Open Architecture
AlphaMon is an Open Architecture Platform that allows software developers, hardware designers and end-users to join the AlphaMon ecosystem.
Whilst we don't share the underlying firmware for security reasons, we do attempt to make it as easy as possible for any interested parties to participate in and enhance the platform offerings, including custom devices based on the AlphaMon's features and services.
You can read more about The AlphaMon Platform on our official website at https://AlphaMon.net

Whilst the firmware is currently tied to the Heltec ESP32 LoRa V3 sub-board we expect to support other OEM ESP32 sub-boards in the near future.
Unlike many SaaS offerings, it is the ESP32 sub-board that joins the AlphaMon ecosystem. Each registered ESP32 receives automatic firmware updates via the built-in Over-The-Air (OTA) update service; much like your cell phone's automatic update service.

If you have an existing Heltec ESP32 LoRa V3 sub-board you can register it in The AlphaMon Platform ecosystem for as little as US$50 per device as an introductory offer, which includes a lifetime subscription.  Other participation plans are available for OEMs and systems integrators. Contact us for more details, clearly stating your preferences, volumes and other requirements for a quotation.
The offical Heltec website for the ESP32 board is here: https://heltec.org/project/wifi-lora-32-v3/


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
- `/configs`: Information on Configuration File settings and Config File management.
