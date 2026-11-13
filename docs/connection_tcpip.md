# tcp/ip

You can read more about the [VSCP tcp/ip link protocol](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_over_tcp_ip) in the VSCP Specification.

## Add a connection

![](./images/add_connection_tcpip.png)

Right-click **tcp/ip connections** in the connection tree and select `Add new tcp/ip connection...`, or use `File/New connection...`.

| Field | Description |
| ----- | ----------- |
| Description | Name of the connection in the connection tree. |
| Host URL | Address of the remote host, `tcp://host:port`. The VSCP daemon listens on port 9598 by default. Use `stcp://` for an SSL/TLS connection. |
| Username / Password | Credentials for the remote host. The password is stored in the keyring of the operating system, see [passwords and keys](passwords_and_keys.md). |
| Interface | Interface to use on the remote host. Gateways often have several interfaces. `Get interfaces` fills the list from the remote host. |
| Connect timeout (ms) | Milliseconds to wait for the connection to be set up (default 15000). Connections saved by earlier versions with a value below 1000 are read as seconds. |
| Response timeout (ms) | Milliseconds to wait for a response to a command. |
| Full Level II (using full GUID) | Address nodes with their full GUID instead of a nickname (no Level I over Level II). |
| Use poll (slow) | Poll the remote host for events instead of using a separate receive connection. Needed for low end devices that allow only one connection; try it if the connection times out. |
| Binary mode | Use the binary protocol after login. See below. |

### SSL/TLS

![](./images/add_connection_tcpip_tls.png)

`SSL/TLS` opens the TLS settings. The same dialog is used by MQTT and websocket connections.

| Field | Description |
| ----- | ----------- |
| SSL/TLS Enable | Use an encrypted connection. |
| SSL/TLS verify peer | Check the certificate of the server. Turn this off only for testing, for example with self-signed certificates. |
| CaFile / CaPath | A file, or a folder, with CA certificates used to verify the server. |
| Cert File / Key File | Client certificate and its private key, if the server requires a client certificate. |
| Password key file | Password for an encrypted private key file. It is stored in the keyring of the operating system. |

### Binary mode and encryption

Check **Binary mode** to use the binary protocol after login instead of the text protocol. The remote host must support binary mode.

In binary mode you can also select **Binary encryption**. All binary frames are then encrypted and authenticated (AES + HMAC-SHA-256). Encryption over tcp/ip is only available in binary mode, and it is always authenticated, so both keys are needed:

| Field | Value |
| ----- | ----- |
| Binary encryption | `AES128`, `AES192` or `AES256` (each with HMAC-SHA-256) |
| Encryption key (hex) | Exactly 32, 48 or 64 hex characters for AES128, AES192 or AES256 |
| HMAC key (hex) | Exactly 64 hex characters (32 bytes), different from the encryption key |

The remote host must use the same algorithm and keys. A frame that fails authentication closes the connection; there is no fallback to unencrypted frames. `Test connection` tests the binary mode and encryption settings as a session uses them.

### Filters
![](./images/add_connection_tcpip_filter.png)

### Test connection
![](./images/add_connection_tcpip_test.png)

### Get Interfaces
![](./images/add_connection_tcpip_interfaces.png)

## Remove a connection

Right-click the connection in the tree view and select `Remove this connection`. The connection is removed.

## Edit a connection

Right-click the connection in the tree view and select `Edit this connection`. A dialog opens where you can change the connection settings.

## Clone a connection

Right-click the connection in the tree view and select `Clone this connection`. A copy of the connection is added, with ` (copy)` appended to its name. Use `Edit this connection` to change the name and settings of the copy.

## Connect to a remote node

Right-click the connection in the tree view and select the window you want to open (session, configuration, node scan or bootloader wizard). The connection is established from that window.

