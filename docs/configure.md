# Configuration

The config file for VSCP works is JSON based and is located in the user home directory. It is named vscp-works-qt.json

| System | Path |
| ------ | ---- |
| Linux | ~/.config/VSCP/vscp-works-qt.json |
| Windows | %APPDATA%\VSCP\vscp-works-qt.json |
| MacOS | ~/Library/Application Support/VSCP/vscp-works-qt.json |

 Care should be taken when editing this file. It is recommended to use the VSCP Works+ application to edit the file.
 
 The configuration file have the following format

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
The prefered language for the application. One of the following

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
If true the application will ask before deleting an object.

### bDarkTheme
Enable a dark theme for the application.

### bSaveAlwaysJson
If true the application will always save configuration as JSON.

### configDisableColors
If true colors are disabled.

### configNumericBase
The numeric base for the application. One of the following

| Base | Description |
| ---- | ----------- |
| 0 | Decimal |
| 1 | Hexadecimal |
| 2 | Octal |
| 3 | Binary |

### configTimeout
The timeout for the application.

### vscpHomeFolder
The VSCP home folder.

## Dimensions settings

### mainwindow-dimensions
The dimensions for the main window. 

## Logging settings

**spdlog** is used and it is a fast and lightweight logging library for C++. It provides flexible formatting options using the `{fmt}` library. The format of log messages can be customized using **format patterns**.  

### **Basic Format Syntax**  
A format pattern consists of **placeholders** enclosed in `{}`. Each placeholder represents a specific log attribute.  

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
The log level for the console. One of the following

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
The log pattern for the console.

### fileLogLevel
The log level for the file. One of the following

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
The maximum number of log files.

### fileLogMaxSize
The maximum size of a log file.

### fileLogPath
The path for the log file.

### fileLogPattern
The log pattern for the file.

### firmwareDeviceCodeRequired
If true the device code is required for firmware updates.

### last-eventdb-download
The timestamp for the last event database download.

## Session settings

### maxSessionEvents
The maximum number of session events.

### numericBase
The numeric base for the application. One of the following

| Base | Description |
| ---- | ----------- |
| 0 | Decimal |
| 1 | Hexadecimal |
| 2 | Octal |
| 3 | Binary |

### sessionAutoConnect
If true the application will auto connect.

### sessionAutoSaveTxRows
If true the application will auto save transmit rows.

### sessionClassDisplayFormat
The class display format for the session. One of the following

| Format | Description |
| ------ | ----------- |
| 0 | Decimal |
| 1 | Hexadecimal |
| 2 | Symbolic |

### sessionGuidDisplayFormat
The GUID display format for the session. One of the following

| Format | Description |
| ------ | ----------- |
| 0 | Decimal |
| 1 | Hexadecimal |
| 2 | Symbolic |

### sessionShowFullTypeToken
If true the application will show the full type token.

### sessionTimeout
The timeout for the session.

### sessionTypeDisplayFormat
The type display format for the session. One of the following

| Format | Description |
| ------ | ----------- |
| 0 | Decimal |
| 1 | Hexadecimal |
| 2 | Symbolic |


## Connection settings

### Common settings

#### type
The type for the connection. One of the following

| Type | Description |
| ---- | ----------- |
| 0 | No connection |
| 1 | tcp/ip connection |
| 2 | CANAL connection |
| 3 | Socketcan connection (Only on Linux) |
| 4 | ws1 connection  |
| 5 | ws3 connection  |
| 6 | MQTT connection |
| 7 | udp connection  |
| 6 | multicast connection |

#### uuid
The UUID for the connection. It has the form {eb0bafcb-8bad-4fcb-b282-09835e3b88eb}

#### name
The name for the connection.



### tcp/ip connection

VSCP over tcp/ip is a connection that is used to connect to a VSCP daemon or an other device that has tcp/ip interface. The protocol is defined [here](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_over_tcp_ip)

#### bfull-l2
If true the connection is full Level II that us using the full GUID to identify devices.

#### bpoll
If true the connection should use polling.

#### btls
If true the connection should use TLS.

#### bverifypeer
If true the connection should verify the peer.

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
The host for the connection on the form "tcp://192.168.1.6:9598"

#### user
The password for the connection.

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
GUID filter for the connection. Should be on the form "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"

##### guid-mask
GUID filter for the connection. Should be on the form "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"

### Interfaces

#### selected-interface
The selected interface for the connection. Should be on the form the tcp/ip host interface present it e.g. "FF:FF:FF:FF:FF:FF:FF:F5:02:88:88:00:00:02:00:00 type=5 (UDP client) obid=2"

#### interfaces

This is an array with the available interfaces presented by the VSCP tcp/ip host. The form is

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

The CANAL connection is used to connect to a VSCP devove or a bus over CANAL (**CAN** **A**bstraction **L**ayer). The API is defined [here](https://docs.vscp.org/#canal). VSCP level I drivers use the CANAL API, and you can find a list of available drivers [here](https://docs.vscp.org/#level1drv). 

The CAN frame format used is described [here](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_over_can_can4vscp)

#### config
The semicolon separated string the connection (the CANAL configuration string).

#### datarate
The data rate for the connection.

#### path
The path to the CANAL driver.

#### response-timeout
The response timeout for the connection.

### Socketcan connection

The Socketcan connection can be used on Linux systems to connect to a CAN bus over the socketcan interface. Just as for the CANAL connection a CAN packet format is used as described [here](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_over_can_can4vscp)

#### device
The device for the connection.

#### response-timeout
The response timeout for the connection.

#### flags
The flags for the connection.

#### filters
The filters for the connection. 




### MQTT connection

#### bfull-l2
If true the connection is full Level II that us using the full GUID to identify devices.

#### btls
If true the connection should use TLS.

#### bverifypeer
If true the connection should verify the peer.

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
The host for the connection. Should be on the form "tcp://192.168.1.7:1883"

#### user
The password for the connection.

#### password
The password for the connection.

#### keepalive
The keep alive for the connection.

#### cleansession
If true the session should be started cleaned.

#### clientid
The client id for the connection. If not set a random client id will be generated.

#### extended-security
If true the connection should use extended security.

#### connection-timeout
The connection timeout for the connection.

#### response-timeout
The response timeout for the connection.

#### publish 
The publish topic settings for the connection. This is one of more topics that can be set on the form

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

The format for the event when it is published on the MQTT topic.

| Format | Description |
| ------ | ----------- |
| 0 | JSON |
| 1 | XML |
| 2 | VSCP string format (CSV)  |
| 3 | VSCP binary format  |

##### qos
The quality of service for the event.

##### bretain
If true the event should be retained.




#### subscribe
The subscribe topic settings for the connection. This is one of more topics that can be set on the form 

```json 
"subscribe": {
    "topic": "vscp/1/2/3",
    "format": 0
}
``` 

where format is one of the following

| Format | Description |
| ------ | ----------- |
| 0 | Auto detect |
| 1 | JSON |
| 2 | XML |
| 3 | VSCP string format (CSV)  |
| 4 | VSCP binary format  |

### ws1 connection

The VSCP websocket connection is used to connect to a VSCP daemon over a websocket connection. ws1 is a string based protocol. The protocol is defined [here](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_websocket?id=ws1-description)

#### user
The password for the connection.

#### password
The password for the connection.

#### url
The URL for the connection on the form "ws://localhost:9598/ws1" or "wss://localhost:9598/ws1"

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
GUID filter for the connection. Should be on the form "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"

##### guid-mask
GUID filter for the connection. Should be on the form "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"


### ws2 connection

The VSCP websocket connection is used to connect to a VSCP daemon over a websocket connection. ws2 is a JSON based protocol. The protocol is defined [here](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_websocket?id=ws2-description)

#### user
The password for the connection.

#### password
The password for the connection.

#### url
The URL for the connection on the form "ws://localhost:9598/ws1" or "wss://localhost:9598/ws1"

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
GUID filter for the connection. Should be on the form "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"

##### guid-mask
GUID filter for the connection. Should be on the form "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"


### udp connection

The UDP connection is used to listen for UDP packets on a specific port. The connection is used to receive events from remote devices. The protocol is defined [here](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_over_udp)

#### ip
The ip-address to listen on for UDP-packets.

#### key
The encryption key for the connection.  

#### encryption
The encryption for the connection. One of the following

| Encryption | Description |
| ---------- | ----------- |
| 0 | None |
| 1 | AES128 |
| 2 | AES192 |
| 3 | AES256 |
| 4 | Selected from byte 0 |

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
GUID filter for the connection. Should be on the form "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"

##### guid-mask
GUID filter for the connection. Should be on the form "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"


### multicast connection

VSCP multicast connection is used to listen for multicast packets on a specific multicast address (24.0.23.158). The connection is used to receive events from remote devices. The protocol is defined [here](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_over_multicast)

#### ip
The multicast IP for the connection. Normally the VSCP multicast reserved ip address (224.0.23.158) is used

#### key
The encryption key for the connection.

#### encryption
The encryption for the connection. One of the following

| Encryption | Description |
| ---------- | ----------- |
| 0 | None |
| 1 | AES128 |
| 2 | AES192 |
| 3 | AES256 |
| 4 | Selected from byte 0 |