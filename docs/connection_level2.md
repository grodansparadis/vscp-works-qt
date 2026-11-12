# Level II driver

A Level II driver connection loads a VSCP Level II driver (`vscpl2drv-*`) directly into VSCP Works and exchanges events with it. No VSCP daemon is needed. Any Level II driver can be used, for example `vscpl2drv-tcpipsrv`, `vscpl2drv-socketcan`, `vscpl2drv-mqtt` or `vscpl2drv-sim`.

Level II drivers are described in the [VSCP specification](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_driver_interfaces).

## Add a connection

| Field | Value |
| ----- | ----- |
| Description | Name of the connection in the connection tree. |
| Driver | Path to the driver shared library, for example `/var/lib/vscp/drivers/level2/libvscpl2drv-tcpipsrv.so`. |
| GUID | GUID given to the driver when it is opened. Usually the two lowest bytes are zero to leave room for node ids. Leave empty for all zeros. |
| Response timeout | Timeout in milliseconds for writing an event to the driver. |

### Driver configuration

Every Level II driver reads a configuration file when it is opened. You can give the configuration in two ways:

* **File** - The path to an existing configuration file. The driver reads it directly.
* **Embedded** - The configuration is stored with the connection in the VSCP Works settings. Use `Load from file...` to start from an existing file, and `Save to file...` to write it out again. When the connection is opened the configuration is written to a new temporary file that only you can read, and the driver is given the path to that file. The file is removed when the connection is closed.

An embedded configuration is usually JSON. It is given to the driver exactly as written. If it looks like JSON it must be valid JSON. A configuration starting with `<` is written to a `.xml` file, JSON to a `.json` file.

Embedded configurations are stored in plain text in the VSCP Works settings, like the passwords of other connection types. Use a configuration file if it holds credentials you do not want stored there.

### Filters
The receive filter decides which events from the driver are shown.

### Test connection
Loads the driver, opens it with the configuration and shows the driver version, then closes it again. Note that opening some drivers has side effects, for example `vscpl2drv-tcpipsrv` starts listening on its port while the test runs.

## Remove a connection

Right-click the connection in the tree view and select `Remove this connection`. The connection is removed.

## Edit a connection

Right-click the connection in the tree view and select `Edit this connection`. A dialog opens where you can change the connection settings.

## Clone a connection

Right-click the connection in the tree view and select `Clone this connection`. A copy of the connection is added, with ` (copy)` appended to its name. Use `Edit this connection` to change the name and settings of the copy.

## Connect to a remote node

Right-click the connection in the tree view and select the window you want to open (session, configuration, node scan). The driver is loaded and opened from that window, and closed when the window is closed. In the configuration window nodes are addressed with a node id, as for CANAL connections.
