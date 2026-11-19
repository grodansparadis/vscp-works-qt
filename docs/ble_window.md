# Bluetooth LE session

The Bluetooth LE session window receives VSCP events that nodes send over Bluetooth Low Energy and talks to a node over a connection. It implements the draft VSCP Bluetooth LE transport with the [vscp-ble](https://github.com/grodansparadis/vscp-ble) library:

| Profile | What | Shown as |
| --- | --- | --- |
| **E1** | One VSCP event in a Bluetooth 5 extended advertisement | `public` |
| **A2** | The same, authenticated and encrypted with AES-128-CCM | `authenticated` (only with a key) |
| **F1** | A large event (223-1024 data octets) split over several advertisements | `public` |
| **L0** | Legacy 31-octet advertising (optional) | `public` |
| **G1** | A connection to the node's VSCP GATT service, events in both directions | `secure link` |

The window needs a Bluetooth adapter. Extended advertising (E1, A2, F1) needs a Bluetooth 5 adapter and, on Linux, BlueZ 5.56 or later.

## Connections

Bluetooth LE connections are listed under **Bluetooth LE Connections** in the connection tree. Create one with `Add new Bluetooth LE connection...` (right-click), by choosing **Bluetooth LE** in the new connection dialog, or with `Bluetooth LE session...` in the **File** menu.

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

`Pause` stops adding events to the list; `Clear` empties it.

On Linux the window gets the advertising data through BlueZ, which reports manufacturer data per Company Identifier. The specification's checks for several VSCP frames in one advertisement and for truncated reports can therefore not be made here.

## G1 connection

Select a device that shows `yes` in the **G1** column and press `Connect G1` (or double-click it). The window

1. connects and finds the VSCP G1 service,
2. reads the node's capabilities and GUID,
3. subscribes to the Control characteristic and negotiates the limits (HELLO),
4. subscribes to the node's events.

G1 needs an encrypted link with authenticated pairing (a passkey), and the node must authorize this computer. Pair first with `Pair` (enter the passkey the node shows), the system Bluetooth settings or `bluetoothctl`. If the node refuses a write, the status says so; pair and connect again.

When the status shows **Ready**, events from the node appear in the event list as `G1`, and `Send event` sends the class, type and data in the G1 box to the node. The sender GUID defaults to `FF:FF:FF:FF:FF:FF:FF:FE` + the adapter address + `00:00`. One event is sent at a time: the next can be sent when the node has answered. A rejected event shows the node's reason (`NOT_AUTHORIZED`, `INVALID_EVENT`, ...).

## Limits

- VSCP Works cannot send E1/A2 broadcasts (the Qt Bluetooth module only does legacy advertising). Use the vscp-ble tools to broadcast, or send to a node over G1.
- macOS hides Bluetooth addresses: devices are shown by an id instead, and L0 GUIDs (which contain the address) can not be formed.
