# Platform overview

## Parts

| Part | Role |
|---|---|
| **AlphaMon node** | ESP32 device that polls Modbus RTU/TCP devices (inverters, meters, batteries) using DDFs and publishes their registers over MQTT. Firmware 4.17 |
| **AlphaMon hub** | An AlphaMon device with a screen (for example the M5Stack Tab5) that also pairs phones, applies signed settings, runs local rules and raises health alarms. It always enforces its own safety limits and keeps working on cached settings when offline |
| **MQTT broker** | Operator-controlled Mosquitto with dynamic security. Devices connect over TLS (8883); phones over WSS. Every login has its own access list (see [mqtt-topics.md](mqtt-topics.md) §5) |
| **Broker gate** | The only server-side login allowed to administer the broker. Other services ask it to create or delete phone logins; it refuses anything that touches device, service or admin logins |
| **AlphaMon server** | HTTPS API and database: accounts, households, sites and members, pairing, settings, audit log, history and health events. It is the only publisher of settings and acknowledgements |
| **Phone app** | Installable web app for households and installers. Live readings come straight from the broker over WSS with a read-only login; every change goes through the server |
| **Home Assistant** (optional) | The hub can publish to the household's own broker with Home Assistant discovery; see [HOMEASSISTANT.md](../../HOMEASSISTANT.md) |

## Data flows

```
                 Modbus RTU/TCP
 Inverter / meter / battery  <──────>  AlphaMon hub or node
                                            │  MQTT over TLS (8883)
                                            ▼
 Phone app  ── WSS, read-only ──────>  MQTT broker  <──── AlphaMon server
     │                                                        ▲
     └──────────── HTTPS (sign-in, settings, pairing) ────────┘
```

- **Live readings:** device → broker → phone. Retained, one value per topic, after DDF scaling.
- **Settings:** phone → server (HTTPS) → broker → hub, signed by the server. The hub reports what it applied; the phone shows "applied" only from that report. See [settings-sync.md](settings-sync.md).
- **Changes on the hub screen or from Home Assistant:** hub → broker → server, which orders and merges them and sends the result back as new settings.
- **History, inventory and health events:** hub → broker → server, acknowledged by the server, replayed by the hub after an outage.
- **Reference data and fleet commands** (prices, weather warnings, VPP events; Proposed): server → broker → hubs, signed.

## Principles

- **Phones never publish.** Their broker logins are read-only; every change goes through the server.
- **The hub enforces its own limits** (battery floor, maximum export, relay defaults), whatever the server, Home Assistant or the screen asks for.
- **No ordering by device clock.** A hub's clock can boot wrong. Settings are ordered by version and by the time the server receives a change. A hub sends timestamps only after NTP has synced.
- **Signed server messages.** Every server-to-hub message on the newer topics is signed (Ed25519), names its own topic and site, and is checked against keys pinned on the hub.
- **Separate keys by role:** a settings key (server), a feeds key (reference data) and a fleet key (kept off the server, each command approved by an operator). Each hub pins two public keys per role, current and next, so keys rotate without a firmware update.
- **Per-site isolation.** Every site has its own broker logins and topic roots; a phone paired to one site can read that site and nothing else.
- **Fail safe.** When a feed expires or the connection is lost, the hub falls back to its own safe behaviour: prices expire and price-driven modes fall back, and no automatic storm mode without a current warning.

## Trust boundaries

| Boundary | Protection |
|---|---|
| Hub ↔ broker | TLS with certificate verification; one login per site; access lists limit each login to its own site's topics |
| Server → hub | Ed25519 signatures with role-split keys; version ordering stops replay of older settings |
| Phone ↔ broker | WSS; per-phone read-only login, removable from the hub screen or the app |
| Phone ↔ server | HTTPS; passkey, Google or email-link sign-in; session cookie (HttpOnly, Secure, SameSite=Lax); exact Origin check on every change |
| Hub storage | Production units MUST enable flash encryption (with NVS encryption) and secure boot, because private keys are kept on the device |

Private data stays out of shared topics: a device's Config File details (WiFi SSID, LAN addresses, broker settings) MUST NOT be published under the site root that phones can read.
