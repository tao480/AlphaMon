# AlphaMon platform architecture

These documents describe how AlphaMon hubs, the MQTT broker, the AlphaMon server and the household phone app fit together, and the messages that pass between them. They are written for integrators, DDF authors and anyone building on the platform. They contain no source code; the hub firmware stays closed source.

| Document | What it covers |
|---|---|
| [platform-overview.md](platform-overview.md) | The parts of the platform, the data flows between them and the trust boundaries |
| [mqtt-topics.md](mqtt-topics.md) | Every MQTT topic, who publishes it, its payload and its access rules |
| [settings-sync.md](settings-sync.md) | How household settings travel from the app to the hub, signed, ordered and confirmed |
| [pairing.md](pairing.md) | How a phone pairs with a hub and gets read-only access to its site |

**Status: draft.** Firmware 4.17 behaviour (live readings, `_Status`) is current. Items marked **Proposed** are agreed with the hub firmware workstream but not yet approved by Solargy, and may change.

**Which hub?** "Hub" means an AlphaMon device that runs the newer hub firmware (for example the M5Stack Tab5 build), which adds pairing, settings sync and health events. Nodes on firmware 4.17 publish live readings as described in [mqtt-topics.md](mqtt-topics.md) §1 and need nothing else.

Keywords MUST, MUST NOT, SHOULD and MAY are normative.
