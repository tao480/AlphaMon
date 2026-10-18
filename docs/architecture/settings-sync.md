# Settings sync

A site's settings are held by the AlphaMon server. The server sends them to the hub signed; the hub applies them within its own limits and reports what it applied. Changes made on the hub screen or from Home Assistant go up to the server, which orders and merges them.

## 1. Flow

1. A household member changes a setting in the app (`PUT /api/sites/:id/settings` with the version they last saw).
2. The server stores it with a higher version and publishes a signed, retained `desired` on `alphamon-settings/<siteId>/desired`.
3. The hub verifies the signature, checks the version is newer, applies the settings within its own limits and publishes a retained `reported` with the version it applied and the fields it capped (`clamped`).
4. The app shows "applied" only once `reported` names the new version.

A change on the hub screen or from Home Assistant applies locally at once (within limits, so it works offline) and goes up as a `change` with a unique `changeId` and the version it was `base`d on. The hub resends a `change` until a `desired` lists its id in `acked`.

## 2. Signed envelope

Every server-to-hub message on the newer topics is wrapped:

```json
{ "body": "<exact JSON string>", "kid": "<key id>", "sig": "<base64 Ed25519 over the UTF-8 bytes of body>" }
```

- `body` names its own `topic` and `siteId` (or `region` / `bomArea`), so a message can't be replayed onto another site or topic.
- The hub accepts a message only if `kid` is one of the keys pinned for that topic root, and rejects unsigned or badly signed messages.
- Keys are split by role: **settings** (the server, `alphamon-settings/…/desired`), **feeds** (reference data, `alphamon-ref/#`) and **fleet** (kept off the server, each command approved by an operator, `alphamon-fleet/#`). The hub pins two public keys per role, current and next, so keys rotate without a firmware update.
- Crypto is libsodium on both ends: Ed25519 signatures, X25519 keys, `crypto_box_seal` for sealed boxes.

## 3. Ordering

- Settings carry `(epoch, version)`. The hub stores the last applied pair in NVS and rejects anything lower. The server never moves backwards; if its data is ever reset and a hub reports a higher pair, the server moves to a higher epoch and republishes.
- Changes are idempotent by `changeId`. If `base` is the current version, the server accepts the change and increments the version. If `base` is older, fields that don't overlap are merged; on a field changed on both sides, the change received later wins and the server records a conflict that the app shows to the household. Either way the result goes out as a new `desired`.
- There is no ordering by device clock: a hub's clock can boot wrong.

## 4. Fields

The hub's limits are its own; the server checks the same ranges, so a reject on the hub is only a safety net (the hub keeps its previous settings). A hub MAY treat a missing field as "keep current" and SHALL ignore a field it doesn't know, so a new field never makes an older hub reject the whole update.

| Field | Values | Notes |
|---|---|---|
| `profile` | `"resilient"` \| `"balanced"` \| `"trader"` | |
| `storm` | `"auto"` \| `"on"` \| `"off"` | |
| `reserveSoc` | integer 10–100 (%) | 10 is the floor because battery makers set a minimum SoC. Stored and ignored with no battery |
| `exportLimitW` | integer 0–30000 (W) | The hub caps it at the site cap (§5) and lists it in `clamped` |
| `pvArrays` | 0–4 entries `{ "wp": 1–100000, "tilt": 0–90, "bearing": 0–359 }` (true north = 0; bearing 0 when tilt is 0); `[]` = no arrays | App only |
| `tariff` | `{ "buy": [bands], "sell": [bands] }`, each 1–6 bands `{ "from": "HH:MM", "c": number }` | App only. `from` is wall-clock time in the site's `tz`, minutes `00` or `30`, the first band at `"00:00"`, strictly increasing; a band runs until the next one (the last until midnight), every day. `c` is cents per kWh including GST, at most 2 decimals: buy 0–200, sell −100–100 (negative when the plan charges for export). `null` = no rates (§6) |
| `tz` | `"Australia/Brisbane"`, `"Australia/Sydney"`, `"Australia/Melbourne"`, `"Australia/Hobart"`, `"Australia/Adelaide"`, `"Australia/Broken_Hill"`, `"Australia/Darwin"` or `"Australia/Perth"` | App only. Used for tariff bands, local midnight and the date of daily health records; the hub maps each name to its own rule, daylight saving included |

Any other value: the hub rejects the update. An object or entry with an unknown key is invalid; new data is always added as a new top-level field.

## 5. Site cap for `exportLimitW`

The DNSP-approved export limit is part of the solar install: the installer sets it in the inverter's own export limitation, and the inverter enforces it whatever AlphaMon asks. `exportLimitW` can only lower export below that. The site cap is the largest value the hub accepts: an optional installer limit on the hub, or else the sum of WMax (SunSpec model 120) over every inverter the hub controls. WMax is kept in the hub's device registry, so the cap is known at boot, including for inverters asleep at night, and is re-read at least hourly. While the cap is unknown, the hub holds its last applied value rather than apply a higher one.

## 6. App-only fields

`pvArrays`, `tariff` and `tz` are set in the app only. The hub shows them read-only and never sends them in a `change`; if one arrives in a `change`, the server removes it, still records and acknowledges the change id, and the next `desired` brings the hub back to the server's value.

- The server leaves each of them out of `desired` until the site has a value, so a hub keeps its own until then. `pvArrays` already on a hub are adopted by the server once, from the first valid non-empty `reported` value.
- `reported` carries the stored value exactly as received (the hub stores the JSON, not floats, so `32.55` comes back as `32.55`).
- Time-of-use rates (more than one band) need a saved `tz`.
- **`tariff: null`** means no rates: the hub drops its stored rates and returns to its default rates, and `reported` carries `null`. Unlike a missing field, `null` never means "keep current". The server sends it after a site is released to a new household, until new rates are saved; the app can't send it. The time zone is kept, since it belongs to the house.
- A changed `pvArrays` value resets the hub's learned yield factor; the same value received again doesn't.
