# tcp/ip

You can read more about the [VSCP tcp/ip link protocol](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_over_tcp_ip) in the VSCP Specification.

## Add a connection

![](./images/add_connection_tcpip.png)

### SSL/TLS

![](./images/add_connection_tcpip_tls.png)

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

