# AlphaMon DDF structure (.417)

Data Definition Files (DDFs) are used by The AlphaMon Platform to describe third-party Modbus devices such as inverters, battery
systems, smart meters, etc.

One or more DDF files are read and loaded from the internal SPIFFS file storage system by the AlphaMon each time it reboots. You can specify
which DDF files you wish the AlphaMon to load by editing the CONFIG_LOAD parameter in your AlphaMon's Config File.  The AlphaMon can load up
to 10 DDFs, which includes some internal DDFs that don't rely on a DEVICE_LOAD parameter.

The CONFIG_LOAD parameter is a simple list of comma separated DDF file names you wish to load at the next boot.  To conserve memory and enhance 
performance, be sure to only specify DDFs of external devices that are physically connected.  However, there's an important exception to this rule
that applies when you're running a local mesh network of AlphaMon devices. A "mesh network" refers to a cluster of AlphaMon devices that trust each
other and which share their data, either over a common WiFi LAN or a long-range LoRa wireless connection.  In some instances, if a remore node which
is part of the mesh doesn't have direct Internet and MQTT access, you can configure another node in the mesh that does have that access to relay the
remote node's data to MQTT.

To implement the data relay functionality, simply edit the CONFIG_LOAD parameter of a node which has Internet access to include the matching DDF name
of the remote Modbus device that you wsh to relay.  This allows the local AlphaMon with Internet access to build the necessary internal memory structures 
that parallel the data collection activities of the remote AlphaMon device.

Whilst you can create your own DDF files using any popular text editor, The AlphaMon Platform's ecosystem has pre-configured DDF files available for
some popular Modbus devices. These pre-configured DDFs are stored on an Over-the_Air (OTA) server that is automatically contacted by each AlphaMon 
during boot-up, provided the AlphaMon has Internet access available via its built-in WiFi hardware.

This remainder of this document descibes the prescribed structure of a DDF file.

## 1. File and naming

- A DDF is a single JSON file with a file name extension of `.417`, being the firmware family it targets (e.g. V4.17.xx).
- To ensure DDF names are unique and to allow embedded device names to be customised, each DDF has a unique name, known as a "slug".
- Slug names are a maximum of 8 characters to ensure compatibility with the ESP32's internal SPIFFS file system.
- The DDF file name consists of the upper-case slug name plus `.417`. 
- The `DEVICE_LOAD` in each AlphaMon's `AlphaMon.cfg` file lists slugs, comma separated. For example: DEVICE_LOAD = "GEN24V9, TS63A1V9".
- At boot AlphaMon syncs each slug, firstly by checking the local microSD card (if connected) and then the OTA server and downloads it if not already in SPIFFS.
- The AlphaMon command `SYS OTA DEVICE <slug_name>` triggers the same download by hand. 
- A slug missing on the server logs `OTA_GetDDF: HTTP Fail: 404` and `File ... not found. SKIP`, and "Device setup" still says OK.
- Inspect a loaded device with `AUDIT DEVICE <slug_name>`. The command takes the slug name; not the Alias name.

## 2. Sample Layout (V4.17.xx)
Each DDF consists of 5 parts:
  1. The header, containing the slug name and version information
  2. The "device" section, describing the device to be connected
  3. The "registers" section, listing each of the Modbus registers to be read, their size, name, role, etc.
  4. The "messages" section which optionally lists the expanded values of any bit-mapped registers.
  5. The optional "HA" section which lists the HomeAssistant Discovery links for selected registers.

For example:

{                                         <- CRLF line endings
  "slug": "GEN24V9",
  "ver": "4.17",
  "rev": "13",
  "patch": "0",
  "device": {...one line...},
  "registers": 
    [
    {...one register per line...},
    {...last register, no trailing comma...}
    ],
  "messages": 
    [
    ],
  "HA":
    [
    "...one Home Assistant entry per line...",
    "...last entry, no trailing comma..."
    ]
}
```


## 3. Section structures:

| Key | Type | Meaning |
|---|---|---|
| `slug` | string | Unique DDF identifier. Must match the file name. |
| `ver` | string | Firmware family, `"4.17"`. |
| `rev` | string | Revision of the DDF, used during boot for file sync. |
| `patch` | string | Patch level, "0", which is NOT checked during file sync. |
| `device` | object | One JSON object. See section 4. |
| `registers` | array | One JSON object per register, see section 5. |
| `messages` | array | Text for coded register values, see section 6. May be empty. |
| `HA` | array | Home Assistant discovery strings, see section 7. |

## 4. `device` object

| Field | Notes |
|---|---|
| `cClass` | "M" or "Modbus" for a Modbus-capable device; or "I" or "Inverter"). Only the first letter is tested during the load. |
| `eType` | 3 inverter, 4 meter. Tunes the polling rate (meters are read more often). It does not choose RTU or TCP. |
| `cUserName`, `cMake`, `cModel` | Used in reporting and audit logs. |
| `cAlias` | Short name used in MQTT and `ALIAS LIST`. |
| `cSerialNo` | Optonal field; can be used to enhance reporting. |
| `ui8SlaveId` | Modbus unit ID. |
| `ui16Baud` | RTU baud rate; default 9600. Ignored for TCP. |
| `ui16TTL_Secs` | Time to Live in seconds. The device is considered disconnected if no activity is detected within the TTL period. |
| `ui32MaxSolar`, `ui32MaxUps`, `ui32MaxGrid` | Rated upper limits in Watts, 0 when unused. Used for calculating percentage values. |
| `ui32LoRaSecs`, `ui32MqttSecs` | Controls LoRa and MQTT polling intervals, expresed in seconds. |

## 5. Register objects

Each register is one line and one Json object. Field order is non-critical.

| Field | Meaning |
|---|---|
| `ui16Addr` | Register address as a hex string, `"0x9C87"`. This is the RTU/TCP protocol's zero based address. |
| `ui8Size` | Number of 16-bit registers read: 1 for a 16-bit value, 2 for 32-bit. Larger sizes (3, 5, 8, 10) permitted for strings or blobs. |
| `ui8FcRead` | Modbus read function, 3 (holding) or 4 (input). |
| `ui8FcWrite` | Write function: 0 none, 3, 6 or 16. Use 0 or 16 for a read-only value; stock meters use both. |
| `ui8Access` | 8 bit, unsigned numeric. Indicates Access Modes: Read-Only (R/O), Read-Write (R/W), etc. |
| `bLookup` | 1 if the raw value is translated through the `messages` array (See section 6). |
| `bSigned` | 1 for a signed value. Changes how the type is stored and displayed (`i16`, not `ui16`). |
| `ui8Encoding` | Future expansion. |
| `ui32SecsMqtt` | Publish interval to MQTT, seconds. Typical: 1 for live values, 5 or 60 for slower ones, 3600 for energy totals. |
| `ui32SecsLoRa` | Publish interval over LoRa, seconds. Typical: 20 to 300 for live values, 86400 for energy totals. |
| `ui8DataType` | How to decode the raw words. 1 = ui32, 2 = i32, 3 = ui16, 4 = i16, 5 = string, 8 and 9 flags or bytes, 12=IP address (unit `ip`), 13 enumeration (used with `bLookup`). |
| `ui8Decimals` | Digits shown after the decimal point in reported values. |
| `fMultiplier` | Scale applied to the raw number. `0.1` turns tenths of a volt into volts. Default = 1.0; use -1.0 to reverse the sign. |
| `cUoM` | Unit text: `V`, `A`, `W`, `Hz`, `kWh`, `%`, `VA`, `VAr`, `PF` and so on. |
| `ui8RegMap` | Role code. Refer tote Register Roles section for the stock values. |
| `ui8PollLoRa` | Bitmap: Pub reg via LoRa? 0=no else POLL_ENABLE_TX (1) or POLL_ENABLE_RX (2) |
| `ui8PollMqtt` | Bitmap: Pub reg via MQTT? 0=no else POLL_ENABLE_TX (1) or POLL_ENABLE_RX (2) |
| `stAlias` | A short name for each register that can be used as short-hand when creating Expressions. |
| `stName` | Long human name. For example, `REG_BATTERY_HOME_R_STATUS`. |
| `stGroup` | Supports register clusters, by type. Used for reporting (only). For eample, `GRID`, `PV`, `INVERTER`, `BATTERY`, `SYSTEM`, `UPS`, etc. |

Notes: 
- The first register listed in the DDF is used to test if the device is physically connected.

## 6. `messages` array
Used to convert bit-mapped register values to text messages. For example, a register with an address of 0x406C that has a read value of 0x02 that corresponds
to a text message of "Sleeping" would be encoded in the DDF as follows:

{"ui16Addr": "0x406C", "ui32RegVal": 2, "cMessage": "Sleeping"}


## 7. `HA` array

One Home Assistant MQTT discovery string per published register. Format is `topic|payload`, with backticks standing in for double quotes inside the payload. 
Supports the following macro expansions for HA compatiility: `$AM_ID$` (the unit's ID) and `$MQTT_ROOT$` (the MQTT root path string).

Example:
"/sensor/$AM_ID$/GEN24V9/AcWatts/config|{`name`: `GEN24V9 AC Power Output`, `uniq_id`: `am_$AM_ID$_GEN24V9_AcWatts`, `stat_t`: `$MQTT_ROOT$$AM_ID$/GEN24V9/state`, `unit_of_meas`: `W`, `dev_cla`: `power`, `stat_cla`: `measurement`, `val_tpl`: `{{ value_json.AcWatts }}`, `dev`: {`ids`: [`$AM_ID$_GEN24V9`], `name`: `Fronius Gen24`, `mdl`: `Gen24`, `mf`: `Fronius`}}",

