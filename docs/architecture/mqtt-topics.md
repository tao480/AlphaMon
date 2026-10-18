# MQTT topics

## 1. Sites and live readings (firmware 4.17)

A **site** is one household's set of AlphaMon devices. Every device at a site uses the same Config File `MQTT_ROOT`, of the form `AlphaMon:<siteId>`. `siteId` is unique per broker and MUST NOT contain `/`, `+`, `#` or spaces. All site data is under `<MQTT_ROOT>/` (written `<root>` below).

All values are retained, plain-text UTF-8, one value per topic, after DDF scaling and in the unit the DDF declares.

| Topic | Example | Meaning |
|---|---|---|
| `<root>/<nodeId>/<device>/<register>` | `AlphaMon:Demo/AM637690/SMILE5/BattSoC` = `97.20` | Live value of one DDF register; `<device>` is the device alias |
| `<root>/<nodeId>/_Status/_LastSeen` | `UTC 26-09-18 16:43:51` | Updated every publish cycle; format `UTC YY-MM-DD hh:mm:ss` |
| `<root>/<nodeId>/_Status/_Online` | `Y` / `N` | Retained last-will: `N` set on connect as the will, then `Y` published |
| `<root>/<nodeId>/_Status/_Firmware`, `_Location`, `_UpTime` | `4.17.11` | Node details |
| `<root>/<nodeId>/<device>/_DDF` | `ALPHA5KW\|4.17\|4` | The DDF behind each device alias: `<slug>\|<ver>\|<rev>` |

- `nodeId` is `AM` plus the last 6 hex digits of the MAC.
- Changed values SHOULD be published within 5 s. Removing a device or node SHOULD clear its retained topics (empty retained payload).
- A reader treats a node as **stale** when `_LastSeen` is more than 30 s old, and **offline** when `_Online` is `N`.
- **Sign convention, the same in every DDF:** grid power positive = import, negative = export; battery power positive = discharge, negative = charge. A node normalises a device that reports the other way.
- Names, units and groups come from the DDF (`stName`, `cUoM`, `stGroup`, `cUserName`, `cMake`, `cModel`, `cClass`); register topics carry values only.

## 2. Pairing (hub)

Under a separate root, so phones can read the whole site subtree without seeing pairing traffic. Payloads are flat `|`-separated strings. Details in [pairing.md](pairing.md).

| Topic | Publisher | Payload | QoS, retained |
|---|---|---|---|
| `alphamon-pairing/<siteId>/offer` | hub | `<64 hex SHA-256 of the code>\|<expiresIn s>`; `0` cancels | 1, no |
| `alphamon-pairing/<siteId>/result` | server | `<phoneId>\|<clientName>\|<paired UTC>` | 1, no |
| `alphamon-pairing/<siteId>/revoke` | hub | `<phoneId>` | 1, no |
| `alphamon-pairing/<siteId>/phones` | hub | comma-separated phone ids the hub keeps; `-` = none | 1, yes |

## 3. Hub topics (JSON)

Server-to-hub messages use the signed envelope described in [settings-sync.md](settings-sync.md) §2.

| Topic | Direction | Payload | QoS, retained |
|---|---|---|---|
| `alphamon-settings/<siteId>/desired` | server → hub | signed; body `{ "topic", "siteId", "epoch", "version", "settings": {…}, "acked": [changeIds] }` | 1, yes |
| `alphamon-settings/<siteId>/reported` | hub → server | `{ "epoch", "applied": <version>, "settings": {…as applied}, "clamped"?: [fields capped by the hub] }` | 1, yes |
| `alphamon-settings/<siteId>/change` | hub → server | `{ "changeId", "epoch", "base": <version>, "changes": {…}, "source": "screen" \| "ha", "ts"? }` | 1, no |
| `alphamon-settings/<siteId>/reset-owner` | hub → server | `{ "resetId", "ts"? }`; resent with the same `resetId` | 1, no |
| `alphamon-history/<siteId>/<nodeId>` | hub → server | one batch of 60 s rows (§4) | 1, no |
| `alphamon-history/<siteId>/<nodeId>/ack` | server → hub | `{ "<dev>": <unix s of the last minute stored>, … }` | 1, yes |
| `alphamon-inventory/<siteId>/<nodeId>` | hub → server | the hub's device registry (§4) | 1, yes |
| `alphamon-health/<siteId>/<nodeId>` | hub → server | health events (§4) | 1, no |
| `alphamon-health/<siteId>/<nodeId>/ack` | server → hub | `{ "seq", "max" }` | 1, yes |
| `alphamon-e2e/<siteId>/<phoneId>` | hub → phone | sealed box (`crypto_box_seal`), base64. **Proposed** | 1, no |
| `alphamon-ref/<region>/price`, `…/forecast` | server → hubs | signed; body `{ "topic", "region", "start", "price" }` (forecast: a list). **Proposed** | 1, yes |
| `alphamon-ref/<bomArea>/warning` | server → hubs | signed; body `{ "topic", "bomArea", "type", "start", "end" }` or a signed `cleared` body. **Proposed** | 1, yes |
| `alphamon-fleet/<region>` | server → hubs | signed; body `{ "topic", "region", "id", "action", "start", "end", "jitterSecs", "limits" }`. **Proposed** | 1, no |

A hub sends timestamps (`ts`) only after NTP has synced. A hub subscribes only to its own region and warning area, even though its access list allows the whole `alphamon-ref/#` tree. Fleet commands are bounded, expire, are started at a random point inside `jitterSecs`, are rejected when replayed, and are ignored until NTP has synced; on expiry or signal loss the hub falls back to self-consumption.

## 4. Payload details

**History upload.** The hub logs each device's aliases every 60 s to SD (UTC) and, after reconnecting, uploads what the server hasn't acknowledged, oldest first, at most 60 rows per batch:

```json
{ "dev": "GEN24", "ddf": "GEN24|4.17|0", "from": "2026-10-13T04:00:00Z", "step": 60,
  "aliases": ["AcWatts", "TotalEnergy"], "rows": [[3526, 10432.17], [3510, null]] }
```

- Row *i* is at `from + i × step` seconds; a gap starts a new batch; `null` = no value that minute.
- Only rows logged after NTP sync are kept or sent.
- The hub moves its "uploaded up to" mark only when the retained `…/ack` says the minute is stored, not on PUBACK (a broker acknowledges a publish its access list denied). The server stores rows idempotently, so a resend is harmless.

**Inventory.** `{ "devices": [ { "dev", "ddf"?, "maker", "model", "serial", "fw", "sunspec"?: [model ids], "ratedW"?, "addr", "firstSeen", "lastSeen" } ] }`, the whole registry. Entries match by serial, or by address when there is none. Published on connect and when an entry is added, removed or changes; `lastSeen` only to the day.

**Health events.** `{ "oldest": <lowest seq the hub still holds>, "events": [ { "seq", "ts", "kind", … } ] }`, 1–20 events, oldest first, at most 16 KB.

| `kind` | Fields |
|---|---|
| `start` | `alarm`, `dev`?, `value`?, `text` (plain UTF-8, at most 200 characters) |
| `end` | `alarm`, `dev`?, `startSeq`? (left out when the start was never numbered), `peak`? |
| `auto` | `action` (an automatic hub action, e.g. `modbus_reconnect`), `dev`?, `alarm`?, `ok` |
| `day` | `date` (local `YYYY-MM-DD`), `forecastWh`?, `importWh`?, `exportWh`?, `devices`: [ { `dev`, `actualWh`, `forecastWh`? } ] (at most 16), sent after local midnight |

- `alarm`, `action` and `dev` match `^[a-z0-9_.-]{1,40}$`; Wh values are non-negative integers. Unknown kinds and fields are ignored.
- Sequence numbers only go up per node. The hub keeps its counter in internal flash and continues from the larger of its own next number and the ack's `max` + 1. After the counter is wiped, it numbers nothing until it has read the ack, or for 10 s after subscribing with none.
- Open alarms survive a reboot. A new `start` for an alarm still open ends the old one. The hub doesn't end and restart the same alarm within 60 s.
- When SD is full the hub drops the oldest events, never the `start` of an open alarm. Anything missing below `oldest` is logged once as lost and the ack moves past it.
- The server stores events idempotently, acknowledges what it rejects or rate-limits, republishes every ack from its database on start, and strips control and bidi-override characters from `text`.

## 5. Access lists

| Broker login | Publish | Subscribe |
|---|---|---|
| Site device (one per site) | `<root>/#`; `alphamon-pairing/<siteId>/offer`, `…/revoke`, `…/phones`; `alphamon-settings/<siteId>/reported`, `…/change`, `…/reset-owner`; `alphamon-inventory/<siteId>/#`, `alphamon-e2e/<siteId>/#`; `alphamon-history/<siteId>/+` and `alphamon-health/<siteId>/+` (one level, so a device can't write an `…/ack`) | `alphamon-pairing/<siteId>/result`; `alphamon-settings/<siteId>/desired`; `alphamon-history/<siteId>/+/ack`, `alphamon-health/<siteId>/+/ack`; `alphamon-ref/#`, `alphamon-fleet/#` |
| Phone (one per paired phone) | nothing | `<root>/#`; `alphamon-e2e/<siteId>/<its phoneId>` |
| Member (remote live view for a signed-in household member) | nothing | `<root>/#` |
| Server | `alphamon-pairing/+/result`; `alphamon-settings/+/desired`; the `…/ack` topics; `alphamon-ref/#`, `alphamon-fleet/#` (separate feeds and fleet publishers) | `alphamon-pairing/+/offer`, `…/revoke`, `…/phones`; `alphamon-settings/+/reported`, `+/change`, `+/reset-owner`; `alphamon-history/+/+`, `alphamon-inventory/+/+`, `alphamon-health/+/+` |
| Broker gate (the only broker admin) | the dynamic-security control topics | the dynamic-security control topics |

On Mosquitto 2.1 with dynamic security, receiving needs its own grant (`publishClientReceive`) for every topic a login subscribes to.

**Security rules**
1. The broker is operator-controlled; production devices MUST NOT publish to a shared test or public broker.
2. Production devices MUST connect over TLS (8883) and verify the broker certificate. Phones use WSS only.
3. No login in production may read every site.
4. Config File details (WiFi SSID, LAN addresses, broker settings) MUST NOT be published under the site root.
5. Pairing codes, code hashes and passwords MUST NOT be logged.
