# Bluetooth LE

A Bluetooth LE connection lets VSCP Works receive VSCP events that nodes send over Bluetooth Low Energy, and talk to a node over a Bluetooth connection. It follows the draft *VSCP over Bluetooth Low Energy* transport in the [VSCP specification](https://grodansparadis.github.io/vscp-doc-spec/#/) and uses the [vscp-ble](https://github.com/grodansparadis/vscp-ble) library, where you also find reference nodes for ESP32, Zephyr and Nordic nRF and tools for Linux.

Bluetooth LE connections are opened in their own [Bluetooth LE session](ble_window.md) window. The session, configuration, node scan and bootloader windows are not available for them.

## What you need

- A Bluetooth adapter. To receive the E1, A2 and F1 broadcasts it must support Bluetooth 5 extended advertising. On Linux check with `bluetoothctl show`: `SupportedSecondaryChannels` must be listed.
- Linux: BlueZ 5.56 or later, and your user must be allowed to use Bluetooth (normally the case on a desktop).
- Windows 10/11 or macOS with Bluetooth. On macOS VSCP Works asks for permission to use Bluetooth the first time.
- VSCP Works built with the Qt Bluetooth module (part of *Qt Connectivity*; `qt6-connectivity-dev` on Debian and Ubuntu). The binary releases include it.

## Profiles

| Profile | How | Security | Data |
| --- | --- | --- | --- |
| **E1** | One event per extended advertisement | None: anyone can send it | 0-222 octets |
| **A2** | One event per extended advertisement, AES-128-CCM encrypted and authenticated, protected against replays | Needs the sender's key | 0-200 octets |
| **F1** | One event split over 2-5 advertisements | None | 223-1024 octets |
| **L0** | Legacy 31-octet advertising (older nodes) | None | 0-8 octets |
| **G1** | A connection to the node's VSCP GATT service; events in both directions | Encrypted link, passkey pairing, authorized by the node | 0-487 octets |

## Add a connection

Right-click **Bluetooth LE Connections** in the connection tree and select `Add new Bluetooth LE connection...`, choose **Bluetooth LE** in the new connection dialog, or select `Bluetooth LE session...` in the **File** menu (which also opens the session window).

| Field | Value |
| ----- | ----- |
| Name | A name for the connection. |
| Company Identifier | The Bluetooth SIG Company Identifier the nodes put in their VSCP frames, for example `0x02E5`. `0xFFFF` is reserved for tests and is what the vscp-ble examples use. Only frames with this identifier are decoded. |
| Adapter | The local Bluetooth adapter to use, or `Default adapter`. |
| G1 node | Optional. The Bluetooth address of a node (`AA:BB:CC:DD:EE:FF`) that `Connect G1` connects to when no device is selected in the session window. |
| Legacy L0 | Also decode legacy version 0 advertising. Their GUID is made from the Bluetooth address and the node id, and changes if the node uses a changing (private) address. |
| A2 keys | One row for each A2 sender you want to receive from (see below). |

The settings are checked when you press `OK`.

### A2 keys

A2 events are only shown when VSCP Works has the key of the sender. Each row holds:

| Column | Value |
| ------ | ----- |
| Key id | The key id the node sends (1-65535). |
| Sender GUID | The GUID of the node. The key is only accepted for this GUID. |
| AES key | The 16-octet AES-128 key as 32 hexadecimal digits. |
| Nonce prefix | The node's 5-octet nonce prefix as 10 hexadecimal digits. |

The values must be the same as those provisioned in the node. `New random key` fills the selected row with a new key and nonce prefix from a secure random source; provision them in the node afterwards (see the vscp-ble documentation for [ESP32](https://github.com/grodansparadis/vscp-ble/blob/master/docs/esp32.md) and [Zephyr/Nordic](https://github.com/grodansparadis/vscp-ble/blob/master/docs/zephyr-nordic.md)). Give every node its own key or at least its own nonce prefix.

The keys are stored in the keyring of the operating system, not in the configuration file, and are never part of exported connections. See [Passwords and keys](passwords_and_keys.md).

VSCP Works remembers the last accepted sequence number of every key (in `ble-replay-{uuid}.json` in its data folder), so a recorded A2 frame that is sent again is rejected, also after a restart. If that file can not be written, A2 events are not shown (fail closed). Delete the file only if the node's key has been replaced.

## Remove, edit, clone, export

Right-click the connection and select `Remove this connection`, `Edit this connection`, `Clone this connection`, `Copy session data to clipboard` or `Save session data to file`, as for other connections. Exported data has no A2 keys; when it is imported the settings dialog opens so you can enter them. The connection type can be given as `ble` or `bluetooth` in imported data.

## Open a session

Double-click the connection, or right-click it and select `Bluetooth LE session`. See [Bluetooth LE session](ble_window.md).

## Pairing for G1

G1 requires an encrypted link with authenticated pairing (a passkey), and the node must authorize this computer. Pair once before connecting:

- with the `Pair` button in the session window (enter the passkey the node shows), or
- in the system Bluetooth settings, or
- on Linux with `bluetoothctl`: `agent KeyboardDisplay`, `default-agent`, `scan le`, `pair AA:BB:CC:DD:EE:FF` and enter the passkey.

"Just Works" pairing without a passkey is not enough: the node refuses G1 writes on such links.

## Security notes

- E1, F1 and L0 events are public. Anyone nearby can send them with any GUID. Use them for information, never to trigger anything that needs security.
- A2 events are authenticated: only a holder of the key can send them, and replays are rejected. They are still broadcasts: there is no delivery guarantee.
- Company Identifier `0xFFFF` and the vscp-ble test key (`000102...0f`) are for tests only.
