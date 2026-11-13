# Solving problems and troubleshooting

## Start with the log

Most problems are written to the log. Open it with `Help/View Log...` in the main window and set the level to `Warning` to see warnings and errors only. If the log does not say enough, set the file log level to `Debug` on the `Logging` tab of `Edit/Settings...`, repeat what failed and look again. See [logging](logging.md).

When VSCP Works is started from a terminal, log entries are also written there (console log level).

## A connection does not work

- Use the `Test` button in the connection settings dialog. It checks that the server, broker or interface can be reached with the settings in the dialog.
- Check host, port, user name and password. Passwords and keys are stored in the keyring of the operating system; after moving to another computer they must be entered again. See [passwords and keys](passwords_and_keys.md).
- For TLS connections, check the certificate files and the `verify peer` setting in the TLS dialog.
- For tcp/ip connections to low end devices, try `Use poll (slow)` in the connection settings. See [tcp/ip](connection_tcpip.md).
- For SocketCAN, the interface must be up (`ip link set can0 up type can bitrate 125000`). A virtual interface (`vcan0`) is useful for testing. See [SocketCAN](connection_socketcan.md).
- For CANAL, the driver path and the configuration string must be valid for the driver. See [CANAL](connection_canal.md).

## No events are shown in the session window

- Check the filter of the connection (`Filter...` in the connection settings) and the session filter selected in the session window. See [filters](connections.md?id=filter) and [session filters](session_window.md?id=filters).
- Check that receiving is not paused.

## The node scan or configuration window does not find a node

- The node must answer register reads. Check the nickname (node id) or GUID and, for tcp/ip, the interface.
- Increase the response timeout in the connection settings for slow links (where the connection type has one).
- The MDF of the node is downloaded from the URL stored in the node's registers. If it can not be downloaded or parsed, the log says why. Check the URL with the [MDF editor](mdf.md) or in the [node scan](scan_window.md) window.

## Reporting a problem

Report problems on the [VSCP Works issue tracker](https://github.com/grodansparadis/vscp-works-qt/issues). Include the version (`Help/About`), the operating system, what you did and the relevant part of the log.

---

${/var/creation-time}  
This document is licensed under [Creative Commons BY 4.0](https://creativecommons.org/licenses/by/4.0/) and can be freely copied, redistributed, remixed, transformed and built upon, as long as you give credit to the author.
