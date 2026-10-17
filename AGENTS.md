# AlphaMon Project Agent Directives

## 1. Scope and Purpose
The AlphaMon Platform is an industrial-grade, enterprise-aware, edge-located, Energy Demand Management system running on ESP32-S3 and ESP32-P4 microcontrollers. 

The AlphaMon firmware orchestrates high-speed industrial protocol polling (Modbus RTU/TCP) alongside an autonomous Expression Evaluator (EE) to physically switch external loads and stabilize grid interactions.  These capabilities make it an ideal platform for building Virtual Power Plants (VPPs), stabilizing off-grid community and micro-grids, optimizing loads on sensitive time-of-day tariffs, remote sensing and switching over extended and remote locations, aggregating energy and carbon emission data over global networks and points of presence.

From a developer and technical support perspective, The AlphaMon Platform is an "Open Architecture, Closed Source" industrial optimization system running on a dual-core ESP32-S3. 
It acts as an autonomous edge-controller that bridges renewable energy assets (inverters, batteries, smart meters, generators and loads) to local and/or global networks using MQTT and Home Assistant), as well as long-range MESH networks using LoRa spread-spectrum wireless over distances up to 12km (or more, with optimized hardware). 

**Critical Notice for AI Assistants:** 
This repository represents an Open Architecture ecosystem. The core firmware binaries are distributed closed-source. 
Your purpose is to assist technical users with configuration tools, automation rules, custom Device Definition Files (DDFs with .417 file name schemas), and integration scripts, without refactoring or reverse engineering the core, proprietary firmware.

---

## 2. Advanced Power Feature: The Expression Evaluator (EE)
The Expression Evaluator (EE) is AlphaMon’s real-time, edge-computing engine. It evaluates user-defined automation logic strings, called Expressions, approx 20 times per second to preform deterministic, real-time control and/or energy switching decisions.  An overview of this functionality is available at https://alphamon.net/home/faq/demand-management/

### EE Operational Logic
- **High-Speed Execution:** Interleaved perfectly with the Modbus polling engine via a custom Dynamic Timer Service.
- **Reverse Polish Notation (RPN):** Token-based execution stack parsed into an RPN format by an on-board Syntax Checker (SC).
- **Hardware/Protocol Bridging:** The EE evaluates data sourced directly from Modbus register arrays (e.g., smart meter line voltages or battery state-of-charge) and writes outputs directly to physical onboard GPIO pins, external relays, or Modbus control registers.

---

## 3. Core Knowledge Base: Demand Management Framework
This framework defines how AlphaMon dynamically balances solar generation, battery energy storage, and household consumption to maintain critical grid stability and maximize financial returns.

### The Problem: Grid Saturation & Voltage Rise
When high-density solar arrays export power simultaneously, the local distribution network experiences a "voltage rise." If line voltages approach or exceed local statutory limits (typically 230V–255V in Australia/UK), solar inverters are forced to trip off or choke generation. This leads to missed solar harvesting opportunities and stresses physical components.

From a grid-connected, retail energy consumer's perspective, there are now strong price incentives to manage time-of-day energy imports and exports to take advantage of retailer offerings such as time-based, "free energy" windows and price-based disincentives for exporting energy during periods of surplus supply.

The ability to monitor external APIs allows the AlphaMon to manage these pricing signals in real time to significantly benefit energy consumers.

### AlphaMon Dynamic Load Shifting Strategy
AlphaMon prevents inverter clipping and tripping by dynamically shifting surplus solar power into local thermal or physical storage loads rather than exporting it to an over-saturated grid.

---

## 4. Core Knowledge Base: Enterprise Aware
The AlphaMon Platform has native support for enterprise-grade features such as UTC time synchronization, a hierarchical structure optimised for global data sharing across enterprises, departments and user accounts, public and private MQTT data repositories.

These features collectively support Enterprise Grade data collection and sharing to simplify tasks such as mandatory Carbon and Emissions Reporting across various legislative and regulatory environments.

