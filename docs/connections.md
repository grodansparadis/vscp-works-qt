# Introduction

In the mainwindow of VSCP Works you have a treeview with all communication channels that are available. This is the place where you add, remove and configure connections to your remote hardware devices. Normally you right click on the treeview and select the operation you want to perform. You can also use the file meny items or the toolbar for the same task. It is perfectly fine for most cases to have several windows open at the same time to a selected communication item. 

# Communication types/protocols

![](./images/connections.png)

VSCP Works can communicate with a diverse number of different hardware devices using many different communication protocols. The following protocols are currently supported:

| Protocol | Description |
|----------|-------------|
| [**CANAL**](https://grodansparadis.github.io/vscp-doc-canal/#/) | The VSCP **CAN** **A**bstraction **L**ayer. This is a VSCP level I communication that is based on a CAN frame. A `CANAL driver`is the same as a `VSCP Level I driver` and this driver is responsible for abstraction from the VSCP world of events to low end frames or simulations. The frame format and other details is described [here](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_over_can_can4vscp)  |
| [**Socketcan**](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_over_can_can4vscp) | The Linux kernel supports the CAN bus via the `socketcan` interface. This is also a VSCP level I communication that is based on a CAN frame. Needless to say only available on Linux systems. |
| [tcp/ip](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_over_tcp_ip) | VSCP tcp/ip link protocol. |
| [MQTT](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_over_mqtt) | VSCP over MQTT.  |
| [ws1](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_websocket?id=ws1-description) | A VSCP websocket protocol that is string based. |
| [wsw](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_websocket?id=ws2-description) | A VSCP websocket protocol that is JSON based. |
| [UDP](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_over_udp) | VSCP connection less UDP communication protocol. |
| [Multicast](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_over_multicast?id=vscp-multicast) | VSCP multicast.  |

# Add a connection

Select the connection type you want to add in the treeview and right click. Select `Add connection` in the context menu. A dialog will open where you can enter the parameters for the connection.

### CANAL

![](./images/add_connection_canal.png)

 - `Description`: Set a descriptive name. This value will be used to identify the connection in the treeview.
 - `Path to driver`: Set the path to the CANAL driver to use. Click `set` to browse for the file. CANAL drivers are the same as `VSCP level I drivers`and on most Linux systems you can find them by default in the _/var/lib/vscp/drivers/level1/_ folder. Windows system usually have them in the folder _appdir/VSCP/drivers/level1. The exact location depends on the installation for the driver.
 - `Device configuration string`: CANAL drivers are configured using a semicolon separated string. This string is deriver dependent. The exact format is described in the driver documentation. Some modern drivers have a wizard that can help you to construct this configuration string. Press `wizard` button to open this dialog.
- `Device flags`: This is a bit mask that can be used to set some flags for the driver. The exact meaning of the bits is driver dependent. The driver documentation will tell you what the bits mean.
- `Datarate`: This is a device specific value for the communication rate. It is usually in bits per second but the exact meaning is driver dependent. The driver documentation will tell you what the value should be. In most cases you can leave this value to the default value zero.

#### Filter

The filter for a CANAL driver is two 32-bit values that are used to filter out messages that are not of interest. The first value is the mask and the second value is the filter. If the mask is zero all messages are passed. If the mask is not zero only messages that match the filter are passed. The filter is a bitwise AND operation between the mask and the message id. If the result is equal to the filter the message is passed. 

Truth table for filter/mask

| Mask bit n | Filter bit n | Incoming event class/type bit n | Accept or reject bit n |
|:------------:|:--------------:|:---------------------------:|:------------------------:|
| 0 | x | x | Accept |
| 1 | 0 | 0 | Accept |
| 1 | 0 | 1 | Reject |
| 1 | 1 | 0 | Reject |
| 1 | 1 | 1 | Accept |

Think of the mask as having ones at positions that are of interest and the filter telling what the value should be for those bit positions that are of interest.

  - So to only accept one class set all mask bits to one and enter the class in filter.
  - To accept all classes set the mask to 0. In this case filter don't care.

![](canal_filter_wizard.png)  

The `Filter wizard`  makes it easy to construct filters for your specific need. Just press the `wizard` button and the dialog will open. Enter the class and type you want to filter on and the wizard will construct the filter and mask for you.


![](id_mask.png)

Press the ID/Mask` button to open the dialog where you can enter the filter and mask manually as a 32-bit value.

![](filter_wizard)

The `wizard button` can give you a view of what events are filtered out an not. Select what you are interested in the right part and transfer  as filter value using `<<` button or vice versa using the `>>` button.

In all above cases the numerical base dialog can be used to set the display numerical base for all values. The default is hexadecimal but you can also use decimal, octal or binary. Numbers can be entered using any base. Just proceed hexadecimals with `0x`, octals with `0o` and binary with `0b`. No prefix will be read as decimal values.

### Socketcan
![](./images/add_connection_socketcan.png)

 - `Description`: Set a descriptive name. This value will be used to identify the connection in the treeview.
 - `Device`: Set the device name for the CAN interface. This is usually something like `can0` or `can1`. The exact name is system dependent. You can find the names in the `/sys/class/net/` folder.
 - `Flags`: Socketcan flags. The flags are used to set some special options for the CAN interface.  Press the `...` button to open a dialog where you can set the flags.
 - `Response timeout`: The time in milliseconds to wait for a response from the device. If no response is received the operation will be aborted.
  - `Filters`: You can set any number of filters for the CAN interface. The filter is a 32-bit value that is used to filter out messages that are not of interest. If the mask is zero all messages are passed. If the mask is not zero only messages that match the filter are passed. The filter is a bitwise AND operation between the mask and the message id. If the result is equal to the filter the message is passed.

#### Test button
Press the `Test connection` button to check if your socketcan settings works.

#### Handling filters
You can use the `Add filter` button to add a filter, `Edit filter` button to edit a selected a filter, `Clone filter` button to clone a selected filter and `Delete filter` button to remove a selected filter.

- Click on a filter item to select it.
- Doubelclick on a filter item to edit it.
- Right click on a filter item to get a context menu where you can edit, clone or delete the filter.

See [CANAL filters](connections.md) for more information about filters.

### tcp/ip


# Remove a connection

Select the connection you want to remove in the treeview and right click. Select `Remove connection` in the context menu. The connection will be removed.

# Edit a connection

Select the connection you want to edit in the treeview and right click. Select `Edit connection` in the context menu. A dialog will open where you can edit the parameters for the connection.

# Connect to a connection

Select the connection you want to connect to in the treeview and right click. Select the service your want (session/configure/scan/firmware load) in the context menu. The connection will be established.