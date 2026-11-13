# CAN raw explorer (SocketCAN frame session)

The CAN raw explorer works with plain CAN frames instead of VSCP events. Use it to look at all traffic on a CAN bus, also traffic that is not VSCP, and to send arbitrary standard, extended or CAN FD frames. It is only available on systems with SocketCAN support (Linux).

## Opening the window

There are three ways to open a CAN raw explorer window:

- Right-click on a **Socketcan** connection in the main window connection tree and select `CAN raw explorer`. The SocketCAN device of that connection is used.
- Right-click on the **Raw CAN Connections** item in the connection tree and select `Add new raw CAN connection...`. A raw CAN connection holds the SocketCAN device name (for example `vcan0` or `can0`), a response timeout, option flags and receive filters. Raw CAN connections can be tested with the `Test connection` button in the dialog.
- Select `SocketCAN frame session...` in the main window **File** menu or on the toolbar.

The window title shows the CAN interface that is used, for example `CAN frame session - can0`.

## Connecting

Press `Connect` on the toolbar to open the CAN interface. The same button changes to `Disconnect` when the interface is open. The badge at the right of the toolbar shows the connection state: green when connected, amber when paused, grey when disconnected and red after an error.

`Pause` stops new frames from being added to the display without closing the interface. Press `Resume` to continue. `Clear` (or `Frame/Clear receive log`) empties the receive log.

## Views

Use the `View` combo box to select how received frames are presented.

### Frame log

Every received or transmitted frame is listed with these columns:

| Column | Content |
| ------ | ------- |
| Time | Time when the frame was received/sent. |
| ID | The CAN identifier. |
| Type | `Data` or `Remote` frame. |
| Format | `STD` (11-bit identifier) or `EXT` (29-bit identifier). |
| Data | The payload as hex bytes, or `<empty>`. |

Frames are also marked with their direction (`RX`/`TX`) and flags (`FD`, `BRS`, `ESI`). Hold the mouse over a row to see a tooltip with frame id, payload and flags.

**Time difference between frames** - Select two or more frames in the log. The status badge then shows the time difference (`Δt`, in µs) between the first and the last of the selected frames.

### ID statistics

Shows one row for each CAN identifier seen on the bus:

| Column | Content |
| ------ | ------- |
| ID | The CAN identifier. |
| Received | Number of frames received with this identifier. |
| Δt last (ms) | Time since the previous frame with this identifier. |
| Δt avg (ms) | Average time between frames with this identifier. |
| Data | Last payload received for this identifier. |

This view is useful to find out how often nodes send frames and to spot nodes that are silent or chatty.

## Sending frames

The `Send frame` group (toggle it with `Frame/Show send frame`) is used to transmit a frame.

- **Frame ID** - Decimal (`291`) or hexadecimal (`0x123`).
- **Payload** - Either hex bytes (`11 22 33` or `112233`) or comma separated values where every value can be written in decimal, hex (`0x11`), octal (`0o10`) or binary (`0b11`), for example `0x11,0o10,0b11,4`.
- **Extended ID (29-bit)** - Use an extended identifier. Standard identifiers are limited to `0x7FF` and extended identifiers to `0x1FFFFFFF`.
- **CAN FD** - Send a CAN FD frame. Valid payload lengths are 0-8, 12, 16, 20, 24, 32, 48 or 64 bytes. A standard CAN frame can hold 0-8 bytes.
- **Bitrate switch (BRS)** and **Error state indicator (ESI)** - CAN FD flags.
- **Remote request** - Send a remote request frame (not allowed for CAN FD).

Press `Send` to transmit the frame. Input is validated before sending and you get a message telling what is wrong if a value is out of range.

## Saved frames

Frames you send often can be saved as named templates in the `Saved frames` group (toggle it with `Frame/Show saved frames`).

- Selecting a template copies it into the send group so it can be inspected or modified before it is sent.
- `Save current` (or `Frame/Save current frame`) - Store the frame currently entered in the send group under a name you choose.
- `Load selected` (or `Frame/Send selected frame`) - Load the selected template into the send group and send it.
- `Delete` (or `Frame/Delete selected frame`) - Remove the selected template.
- `Frame/Save frames to disk` and `Frame/Reload saved frames` - Store/reload the template list.

Templates are saved automatically when they are added or deleted and are loaded when the window opens. They are stored per CAN interface in the application settings, so `can0` and `vcan0` have separate lists.

## ID filters

Select `Frame/ID filters...` to open the CAN ID filter dialog. Add one or more ranges (`From ID` - `To ID`) and check `Use` for the ranges that should be active. Only frames with an identifier inside an active range are shown. Use `Add range` and `Remove selected` to edit the list.

## Help

The `Help` button opens this page.
