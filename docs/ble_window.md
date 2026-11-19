# Bluetooth LE session

The Bluetooth LE session window receives VSCP events that nodes send over Bluetooth Low Energy and talks to a node over a connection. It implements the draft VSCP Bluetooth LE transport with the [vscp-ble](https://github.com/grodansparadis/vscp-ble) library:

| Profile | What | Shown as |
| --- | --- | --- |
| **E1** | One VSCP event in a Bluetooth 5 extended advertisement | `public` |
| **A2** | The same, authenticated and encrypted with AES-128-CCM | `authenticated` (only with a key) |
| **F1** | A large event (223-1024 data octets) split over several advertisements | `public` |
| **L0** | Legacy 31-octet advertising (optional) | `public` |
| **G1** | A connection to the node's VSCP GATT service, events in both directions | `secure link` |

The window also shows all other Bluetooth LE advertising (beacons, sensors, phones, ...) and can send general advertising of its own, see [All advertising](#all-advertising) and [Sending advertising](#sending-advertising).

The window needs a Bluetooth adapter. Extended advertising (E1, A2, F1) needs a Bluetooth 5 adapter and, on Linux, BlueZ 5.56 or later.

## Connections

See [Bluetooth LE connections](connection_ble.md) for the settings, A2 keys and pairing. Bluetooth LE connections are listed under **Bluetooth LE Connections** in the connection tree. Create one with `Add new Bluetooth LE connection...` (right-click), by choosing **Bluetooth LE** in the new connection dialog, or with `Bluetooth LE session...` in the **File** menu.

| Setting | |
| --- | --- |
| Name | Name of the connection |
| Company Identifier | The Bluetooth SIG Company Identifier of the VSCP frames. `0xFFFF` is for tests only: products use an assigned identifier |
| Adapter | The local Bluetooth adapter, or the default one |
| G1 node | Optional address of a node to connect to with `Connect G1` when no device is selected |
| Legacy L0 | Also decode legacy (version 0) advertising |
| A2 keys | The keys of the A2 senders: key id, the sender's GUID, the 16-octet AES key and the 5-octet nonce prefix. `New random key` makes a new key and prefix that must then be provisioned in the node |

A2 keys are kept in the system keyring like other [passwords and keys](passwords_and_keys.md). They are never part of exported connection data.

## Receiving

Scanning starts when the window opens; `Scan` stops and starts it. The upper list shows the devices that send VSCP frames or offer the G1 service, the lower list the received events with time, device, profile, trust, GUID, class, type, data and details (sequence number, key and epoch, ...).

- **public** events (E1, F1, L0) can be sent by anyone. Never act on them for anything that needs security.
- **authenticated** A2 events were verified with a provisioned key. A frame that does not verify, uses an unknown or wrong key, or repeats an older sequence number (a replay) is not shown. The last accepted sequence number of each key is saved, so replays are also rejected after a restart.

`Pause` stops adding to the lists (advertising is still received); `Clear` empties the event and advertising lists.

On Linux the window gets the advertising data through BlueZ, which reports manufacturer data per Company Identifier. The specification's checks for several VSCP frames in one advertisement and for truncated reports can therefore not be made here.

## All advertising

The **All advertising** tab (next to **VSCP events**) lists the advertising of every device, not only VSCP frames. Each row is one part of an advertisement:

| Kind | Company / UUID | Data |
| --- | --- | --- |
| Manufacturer | The Company Identifier, with the company name for well-known ones (`0x004C Apple`) | The manufacturer data |
| Service data | The service UUID (`FEAA`, or a 128-bit UUID) | The service data |
| Services | | The advertised service UUIDs (in **Decoded**) |
| Name | | A device that only advertises its name |

The columns are time, device (address), name, RSSI, kind, company / UUID, data (hex) and decoded. **Decoded** explains data the window knows:

- VSCP frames (E1, A2, F1 fragments and L0) with the connection's Company Identifier
- Apple iBeacon: UUID, major, minor and tx power
- Eddystone (service `FEAA`): UID (namespace and instance), URL and TLM (battery voltage, temperature, advertisement count and uptime)

Devices repeat their advertising many times per second. **Only changed data** (on by default) adds a row only when a device's data for that kind and company / UUID changes; turn it off to see every report. The **Filter** field shows only the rows that contain the text in the address, name, company / UUID, data or decoded column (case insensitive).

## Sending advertising

The **Advertise** box sends general legacy (non-connectable) advertising from the computer. Fill in what to send:

| Field | |
| --- | --- |
| Flags (LE General Discoverable) | Add a Flags AD structure (`02 01 06`) |
| Local name | Optional; shortened if it does not fit |
| Company / Manufacturer data | A Bluetooth SIG Company Identifier (for example `0x004C`; `0xFFFF` is for tests only) and the manufacturer data as hex octets (`01 02 03`) |
| Service UUID / Service data | A 16-bit (`FEAA`), 32-bit or 128-bit service UUID and its data as hex octets |
| Raw advertising data | Instead of the fields above, the complete advertising data as AD structures (length, type, value), for example `02 01 06 05 FF FF FF 01 02` |
| Interval | 100-10240 ms (non-connectable advertising sends at most every 100 ms) |

The line under the fields shows the advertising data that will be sent and its size; legacy advertising holds at most 31 octets. An error (bad hex, a too long advertisement, manufacturer data without a Company Identifier, ...) is shown there instead and `Start advertising` is disabled. `Stop advertising` stops it, and it also stops when the window is closed. The window's own advertising shows up in **All advertising** of another computer or phone.

What can be sent depends on the platform:

- **Linux**: everything; the data is sent exactly as shown. BlueZ only lets programs with the `CAP_NET_ADMIN` capability advertise. Without it `Start advertising` fails with "advertising on Linux needs the CAP_NET_ADMIN capability". Give it to the program with

  ```bash
  sudo setcap cap_net_admin+eip /path/to/vscp-works-qt
  ```

  (again after each new build or update), or run it as root (not recommended).
- **macOS**: only the local name is advertised; other data is not sent.
- **Windows**: the Qt Bluetooth module can not advertise, so the box is disabled.

## G1 connection

Select a device that shows `yes` in the **G1** column and press `Connect G1` (or double-click it). The window

1. connects and finds the VSCP G1 service,
2. reads the node's capabilities and GUID,
3. subscribes to the Control characteristic and negotiates the limits (HELLO),
4. subscribes to the node's events.

G1 needs an encrypted link with authenticated pairing (a passkey), and the node must authorize this computer. Pair first with `Pair` (enter the passkey the node shows), the system Bluetooth settings or `bluetoothctl`. If the node refuses a write, the status says so; pair and connect again.

When the status shows **Ready**, events from the node appear in the event list as `G1`, and `Send event` sends the class, type and data in the G1 box to the node. The sender GUID defaults to `FF:FF:FF:FF:FF:FF:FF:FE` + the adapter address + `00:00`. One event is sent at a time: the next can be sent when the node has answered. A rejected event shows the node's reason (`NOT_AUTHORIZED`, `INVALID_EVENT`, ...).

## Theme

`Dark theme` / `Light theme` in the toolbar switches the window between a dark and a light theme, like the other session windows. The choice is saved with the connection; new windows start with the default theme from the settings.

## Limits

- VSCP Works cannot send E1/A2/F1 broadcasts (the Qt Bluetooth module only does legacy advertising, see [Sending advertising](#sending-advertising)). Use the vscp-ble tools to broadcast VSCP events, or send to a node over G1.
- macOS hides Bluetooth addresses: devices are shown by an id instead, and L0 GUIDs (which contain the address) can not be formed.
