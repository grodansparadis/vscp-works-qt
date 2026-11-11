# Configuration

The configuration file for VSCP Works is a JSON file named `vscp-works-qt.json`, located in the user's home directory:

| System | Path |
| ------ | ---- |
| Linux | ~/.config/VSCP/vscp-works-qt.json |
| Windows | %APPDATA%\VSCP\vscp-works-qt.json |
| MacOS | ~/Library/Application Support/VSCP/vscp-works-qt.json |

Take care if you edit this file by hand. We recommend changing settings from within VSCP Works+ instead.
 
 The configuration file has the following format:

```json
{
    "PreferredLanguage": "en",
    "bAskBeforeDelete": false,
    "bDarkTheme": false,
    "bSaveAlwaysJson": false,
    "configDisableColors": false,
    "configNumericBase": 0,
    "configTimeout": 5000,
    "connections": [
        {
            "device": "can0",
            "filters": [],
            "flags": 0,
            "name": "can0",
            "response-timeout": 3,
            "type": 3,
            "uuid": "{2585a211-3cad-45e0-b2c2-c2ee791fb074}"
        }
    ],
    "consoleLogLevel": 1,
    "consoleLogPattern": "%c [%^%l%$] %v",
    "fileLogLevel": 0,
    "fileLogMaxFiles": 10,
    "fileLogMaxSize": 5242880,
    "fileLogPath": ".local/share/VSCP/vscpworks+/logs/vscpworks.log",
    "fileLogPattern": "%c [%^%l%$] %v",
    "firmwareDeviceCodeRequired": false,
    "last-eventdb-download": 1737303607741,
    "mainwindow-dimensions": [
        0,
        0,
        0,
        0
    ],
    "maxSessionEvents": -1,
    "numericBase": 0,
    "sessionAutoConnect": true,
    "sessionAutoSaveTxRows": true,
    "sessionClassDisplayFormat": 0,
    "sessionGuidDisplayFormat": 2,
    "sessionShowFullTypeToken": false,
    "sessionTimeout": 1000,
    "sessionTypeDisplayFormat": 0,
    "vscpHomeFolder": "/var/lib/vscp"
}
```

## General settings

### PreferredLanguage 
The preferred language for the application. One of the following:

| Language | Code |
| -------- | ---- |
| English | en |
| Swedish | sv |
| German | de |
| Spanish | es |
| French | fr |
| Italian | it |
| Dutch | nl |
| Polish | pl |
| Portuguese | pt |
| Russian | ru |
| Chinese | zh |
| Japanese | ja |
| Korean | ko |
| Turkish | tr |
| Arabic | ar |
| Hebrew | he |
| Hindi | hi |
| Bengali | bn |
| Thai | th |
| Vietnamese | vi |

### bAskBeforeDelete
If true, the application asks for confirmation before deleting an object.

### bDarkTheme
Enables a dark theme for the application.

### bSaveAlwaysJson
If true, the application always saves register configurations as JSON.

### configDisableColors
If true, register colors defined in the MDF are not used in the configuration window.

### configNumericBase
The numerical base used in the configuration window. One of the following:

| Base | Description |
| ---- | ----------- |
| 0 | Hexadecimal |
| 1 | Decimal |
| 2 | Octal |
| 3 | Binary |

### configTimeout
The response timeout, in milliseconds, used by the configuration window and node scan.

### vscpHomeFolder
The VSCP home folder.

## Dimensions settings

### mainwindow-dimensions
The size and position of the main window.

## Logging settings

Logging uses **spdlog**, a fast and lightweight logging library for C++. The format of log messages can be customized with **format patterns**.  

### **Basic Format Syntax**  
A format pattern is text mixed with **flags** that start with `%`. Each flag is replaced by a specific log attribute.  

#### **Common Format Flags**  
| Placeholder  | Description |
|-------------|-------------|
| `%+`        | Default format (`[%Y-%m-%d %H:%M:%S.%e] [log_level] [logger_name] message`) |
| `%Y`        | Year (4 digits) |
| `%m`        | Month (01-12) |
| `%d`        | Day (01-31) |
| `%H`        | Hour (00-23) |
| `%M`        | Minute (00-59) |
| `%S`        | Second (00-59) |
| `%e`        | Millisecond (000-999) |
| `%f`        | Microsecond (000000-999999) |
| `%F`        | Nanosecond (000000000-999999999) |
| `%L`        | Three-letter log level (e.g., `INF`, `ERR`) |
| `%l`        | Full log level name (e.g., `info`, `error`) |
| `%t`        | Thread ID |
| `%n`        | Logger name |
| `%v`        | Log message |
| `%P`        | Process ID |
| `%s`        | Short filename (without path) |
| `%g`        | Full filename (with path) |
| `%#`        | Line number |
| `%!`        | Function name |
| `%^`        | Begin color range |
| `%$`        | End color range |

#### JSON logging

``` 
R"({"timestamp":"%Y-%m-%d %H:%M:%S.%e", "level":"%l", "message":"%v"})"
```


### consoleLogLevel
The log level for the console. One of the following:

| Level | Description |
| ----- | ----------- |
| 0 | Off |
| 1 | Critical |
| 2 | Error |
| 3 | Warning |
| 4 | Info |
| 5 | Debug |
| 6 | Trace |

### consoleLogPattern
The log pattern used for the console.

### fileLogLevel
The log level for the log file. One of the following:

| Level | Description |
| ----- | ----------- |
| 0 | Off |
| 1 | Critical |
| 2 | Error |
| 3 | Warning |
| 4 | Info |
| 5 | Debug |
| 6 | Trace |

### fileLogMaxFiles
The maximum number of rotated log files to keep.

### fileLogMaxSize
The maximum size of a log file, in bytes, before it is rotated.

### fileLogPath
The path to the log file.

### fileLogPattern
The log pattern used for the log file.

### firmwareDeviceCodeRequired
If true, the device firmware code must match before firmware is loaded.

### last-eventdb-download
The time of the last download of the event database.

## Session settings

### maxSessionEvents
The maximum number of events kept in a session window. -1 means no limit.

### numericBase
The default numerical base for the application. One of the following:

| Base | Description |
| ---- | ----------- |
| 0 | Hexadecimal |
| 1 | Decimal |
| 2 | Octal |
| 3 | Binary |

### sessionAutoConnect
If true, a session window connects automatically when it is opened.

### sessionAutoSaveTxRows
If true, transmit rows are saved when a session window closes and loaded when it opens.

### sessionClassDisplayFormat
How the VSCP class is displayed in session windows. One of the following:

| Format | Description |
| ------ | ----------- |
| 0 | Symbolic |
| 1 | Numerical in the selected numerical base |
| 2 | Numerical, hex/decimal |
| 3 | Symbolic + hex/decimal |

### sessionGuidDisplayFormat
How the GUID is displayed in session windows. One of the following:

| Format | Description |
| ------ | ----------- |
| 0 | GUID |
| 1 | Symbolic if possible |
| 2 | Symbolic + GUID |
| 3 | GUID + Symbolic |

### sessionShowFullTypeToken
If true, the full type token is shown (the `VSCP_TYPE_` prefix is kept).

### sessionTimeout
The response timeout, in milliseconds, used by session windows.

### sessionTypeDisplayFormat
How the VSCP type is displayed in session windows. One of the following:

| Format | Description |
| ------ | ----------- |
| 0 | Symbolic |
| 1 | Numerical in the selected numerical base |
| 2 | Numerical, hex/decimal |
| 3 | Symbolic + hex/decimal |


## Connection settings

### Common settings

#### type
The connection type. One of the following:

| Type | Description |
| ---- | ----------- |
| 0 | No connection |
| 1 | tcp/ip connection |
| 2 | CANAL connection |
| 3 | Socketcan connection (Only on Linux) |
| 4 | ws1 connection  |
| 5 | ws2 connection  |
| 6 | MQTT connection |
| 7 | udp connection  |
| 8 | multicast connection |

#### uuid
The UUID of the connection, in the form {eb0bafcb-8bad-4fcb-b282-09835e3b88eb}

#### name
The name of the connection.



### tcp/ip connection

A tcp/ip connection is used to connect to a VSCP daemon or another device with a tcp/ip interface. The protocol is defined [here](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_over_tcp_ip)

#### bfull-l2
If true, the connection is full Level II, which uses the full GUID to identify devices.

#### bpoll
If true, the connection uses polling.

#### btls
If true, the connection uses TLS.

#### bverifypeer
If true, the peer certificate is verified.

#### cafile
The CA file for the connection.

#### capath
The CA path for the connection.

#### certfile
The certificate file for the connection.

#### keyfile
The key file for the connection.

#### pwkeyfile
The password for the key file.


#### host 
The host for the connection, in the form "tcp://192.168.1.6:9598"

#### user
The username for the connection.

#### password
The password for the connection.

#### port
The port for the connection.

#### host
The host for the connection.

#### connection-timeout
The connection timeout for the connection.

#### response-timeout
The response timeout for the connection.

#### Filter

##### priority-filter
Priority filter for the connection.

##### priority-mask
Priority mask for the connection.

##### class-filter
Class filter for the connection.

##### class-mask
Class mask for the connection.

##### type-filter
Type filter for the connection.

##### type-mask
Type mask for the connection.

##### guid-filter
GUID filter for the connection. in the form "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"

##### guid-mask
GUID mask for the connection. in the form "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"

### Interfaces

#### selected-interface
The selected interface for the connection, in the form the tcp/ip host presents it, for example "FF:FF:FF:FF:FF:FF:FF:F5:02:88:88:00:00:02:00:00 type=5 (UDP client) obid=2"

#### interfaces

An array of the interfaces presented by the VSCP tcp/ip host, in the form

```json
"interfaces": [
                {
                    "if-item": "FF:FF:FF:FF:FF:FF:FF:F5:02:88:88:00:00:01:00:00 type=5 (UDP client) obid=1"
                },
                {
                    "if-item": "FF:FF:FF:FF:FF:FF:FF:F5:02:88:88:00:00:02:00:00 type=5 (UDP client) obid=2"
                }
            ],
```


### CANAL connection

A CANAL connection is used to connect to a VSCP device or a bus through CANAL (**CAN** **A**bstraction **L**ayer). The API is defined [here](https://docs.vscp.org/#canal). VSCP Level I drivers use the CANAL API. A list of available drivers is [here](https://docs.vscp.org/#level1drv). 

The CAN frame format is described [here](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_over_can_can4vscp)

#### config
The semicolon-separated configuration string for the driver (the CANAL configuration string).

#### datarate
The data rate for the connection.

#### path
The path to the CANAL driver.

#### response-timeout
The response timeout for the connection.

### Socketcan connection

A SocketCAN connection is used on Linux to connect to a CAN bus through the socketcan interface. As for CANAL connections, the CAN frame format described [here] is used(https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_over_can_can4vscp)

#### device
The CAN device, for example `can0`.

#### response-timeout
The response timeout for the connection.

#### flags
The flags for the connection.

#### filters
The filters for the connection. 




### MQTT connection

#### bfull-l2
If true, the connection is full Level II, which uses the full GUID to identify devices.

#### btls
If true, the connection uses TLS.

#### bverifypeer
If true, the peer certificate is verified.

#### cafile
The CA file for the connection.

#### capath
The CA path for the connection.

#### certfile
The certificate file for the connection.

#### keyfile
The key file for the connection.

#### pwkeyfile
The password for the key file.

#### host
The host for the connection, in the form "tcp://192.168.1.7:1883"

#### user
The username for the connection.

#### password
The password for the connection.

#### keepalive
The keep-alive interval for the connection.

#### cleansession
If true, a clean session is used.

#### clientid
The client id for the connection. If it is not set, a random client id is generated.

#### extended-security
If true, the connection uses extended security.

#### connection-timeout
The connection timeout for the connection.

#### response-timeout
The response timeout for the connection.

#### publish 
The publish topic settings for the connection. One or more topics in the form

```json 
"publish": {
    "topic": "vscp/1/2/3",
    "format": 0,
    "qos": 0,
    "bretain": false
}
```

##### topic
The MQTT topic for the event.

##### format

The format of the event when it is published on the topic.

| Format | Description |
| ------ | ----------- |
| 0 | JSON |
| 1 | XML |
| 2 | VSCP string format (CSV)  |
| 3 | VSCP binary format  |

##### qos
The quality of service (QoS) for the event.

##### bretain
If true, the event is published as retained.




#### subscribe
The subscribe topic settings for the connection. One or more topics in the form 

```json 
"subscribe": {
    "topic": "vscp/1/2/3",
    "format": 0
}
``` 

where format is one of the following:

| Format | Description |
| ------ | ----------- |
| 0 | Auto detect |
| 1 | JSON |
| 2 | XML |
| 3 | VSCP string format (CSV)  |
| 4 | VSCP binary format  |

### ws1 connection

A ws1 connection is used to connect to a VSCP daemon over websocket. ws1 is a string-based protocol. The protocol is defined [here](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_websocket?id=ws1-description)

#### user
The username for the connection.

#### password
The password for the connection.

#### url
The URL for the connection, in the form "ws://localhost:9598/ws1" or "wss://localhost:9598/ws1"

#### Filter

##### priority-filter
Priority filter for the connection.

##### priority-mask
Priority mask for the connection.

##### class-filter
Class filter for the connection.

##### class-mask
Class mask for the connection.

##### type-filter
Type filter for the connection.

##### type-mask
Type mask for the connection.

##### guid-filter
GUID filter for the connection. in the form "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"

##### guid-mask
GUID mask for the connection. in the form "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"


### ws2 connection

A ws2 connection is used to connect to a VSCP daemon over websocket. ws2 is a JSON-based protocol. The protocol is defined [here](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_websocket?id=ws2-description)

#### user
The username for the connection.

#### password
The password for the connection.

#### url
The URL for the connection, in the form "ws://localhost:9598/ws1" or "wss://localhost:9598/ws1"

#### Filter

##### priority-filter
Priority filter for the connection.

##### priority-mask
Priority mask for the connection.

##### class-filter
Class filter for the connection.

##### class-mask
Class mask for the connection.

##### type-filter
Type filter for the connection.

##### type-mask
Type mask for the connection.

##### guid-filter
GUID filter for the connection. in the form "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"

##### guid-mask
GUID mask for the connection. in the form "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"


### udp connection

A UDP connection listens for UDP packets on a specific port and is used to receive events from remote devices. The protocol is defined [here](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_over_udp)

#### ip
The IP address to listen on for UDP packets.

#### key
The encryption key for the connection.  

#### encryption
The encryption used by the connection. One of the following:

| Encryption | Description |
| ---------- | ----------- |
| 0 | None |
| 1 | AES128 |
| 2 | AES192 |
| 3 | AES256 |
| 4 | Selected by byte 0 |

#### Filter

##### priority-filter
Priority filter for the connection.

##### priority-mask
Priority mask for the connection.

##### class-filter
Class filter for the connection.

##### class-mask
Class mask for the connection.

##### type-filter
Type filter for the connection.

##### type-mask
Type mask for the connection.

##### guid-filter
GUID filter for the connection. in the form "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"

##### guid-mask
GUID mask for the connection. in the form "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"


### multicast connection

A multicast connection listens for multicast packets on a specific multicast address (normally 224.0.23.158) and is used to receive events from remote devices. The protocol is defined [here](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_over_multicast)

#### ip
The multicast IP address for the connection. Normally the address reserved for VSCP multicast (224.0.23.158) is used.

#### key
The encryption key for the connection.

#### encryption
The encryption used by the connection. One of the following:

| Encryption | Description |
| ---------- | ----------- |
| 0 | None |
| 1 | AES128 |
| 2 | AES192 |
| 3 | AES256 |
| 4 | Selected by byte 0 |