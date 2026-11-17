# VSCP Link protocol test tool

The VSCP Link protocol test tool verifies that a server talking the [VSCP tcp/ip link protocol](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_over_tcp_ip) (for example the VSCP daemon or a VSCP node with a tcp/ip interface) behaves as expected. Use it when you set up a new server, when you develop firmware with a VSCP link interface or when you are hunting connection problems.

Open the tool from the main window menu `Tests/VSCP Link protocol test tool...`.

## Selecting a connection

The `Connection` combo lists all **tcp/ip** connections defined in the main window, sorted by name. If no tcp/ip connection exists, press `Create TCP/IP interface...` to define one directly from the test tool. Press `Refresh` to reload the list after connections have been added or changed in the main window.

The `Selected connection information` group shows the settings that will be used:

| Field | Description |
| ----- | ----------- |
| Name / Type | Connection name and type. |
| Server / endpoint | Host and port. |
| Username | User used for login. |
| SSL/TLS | Whether TLS is enabled. |
| Connection timeout | Timeout used when connecting. |
| Protocol settings | Selected interface, poll mode, full level II and binary mode settings. |
| State | `Connected` or `Disconnected`. |
| Server version | Filled in after the version has been queried. |
| Server capabilities | Filled in after the capabilities have been queried. |

## Test steps

The steps table lists the individual tests. Each row has a step name, the command used, the result (`Not run`, `PASS` or `FAIL`) and details.

| Step | What is tested |
| ---- | -------------- |
| Initialize client | A client can be created from the connection settings. |
| Connect (wait cursor) | The client can connect and log in to the server. |
| Verify connected state | The client reports an active connection. |
| Query server version | The `version` command returns a server version. |
| Query server capabilities | Full level II support and available interfaces are reported. |
| Report unavailable commands | Lists link protocol commands that cannot be tested from this tool. |
| Test command responses | Commands (version, interface listing) return valid responses. |
| Verify full level II support | The connection reports full level II capability. |
| Disconnect | The client can disconnect cleanly. |
| Verify disconnected state | The client reports that it is disconnected. |
| Test binary protocol (no encryption) | Opens a separate connection in binary mode without encryption and verifies a version response and clean disconnect. |
| Test binary protocol (authenticated encryption) | Opens a separate connection using the selected connection's binary encryption algorithm and keys, then verifies a version response and clean disconnect. |

- `Run selected steps` - Run the steps selected in the table. Select several rows to run them in order.
- `Run all steps` - Run the complete flow from initialization to disconnect.

When a step fails, the log lists **hints** about probable causes, such as an unreachable host, wrong credentials, a very low timeout, interface listing being disabled on the server or a stale connection after a failed disconnect.

The binary protocol tests apply to the selected TCP/IP connection and use its host, credentials, TLS and timeout settings. The unencrypted test forces binary mode with encryption disabled, regardless of the connection's saved binary-mode setting. To run the authenticated-encryption test, enable binary mode and configure a supported encryption algorithm, AES key and HMAC key in the selected connection. If no binary encryption algorithm is configured, that test is marked `SKIP` and the remaining tests continue.

## Reliability test

`Run reliability test` repeatedly connects to and disconnects from the server to find intermittent problems.

| Setting | Description |
| ------- | ----------- |
| Cycles | Number of connect/disconnect cycles (1-10000, default 25). |
| Retries | How many extra attempts are made when a connect or disconnect fails (0-20, default 2). |
| Stress events/cycle | Number of events to send in each cycle (0-10000, default 10). Event sending is currently not available in the tool, so these are counted as skipped. |
| Validate connect timeout | Mark a cycle as failed if connecting takes longer than the connection timeout. |

A progress dialog is shown while the test runs, and you can cancel the test. When it is done, the result is shown as:

```
Reliability stats: pass=N fail=N timeout-failures=N stress-send-ok=N stress-send-skipped=N
```

## Log

All actions and results are written to the log area at the bottom of the window. `Clear log` empties it.
