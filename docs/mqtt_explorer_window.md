# MQTT raw explorer

The MQTT raw explorer is a general MQTT tool. It shows any MQTT traffic on a broker, not only VSCP events, organized as a topic tree, and it lets you publish arbitrary payloads. It is useful for looking at what is really published on a broker, for finding topics used by VSCP nodes and for debugging MQTT based setups.

## Opening the window

- Right-click on an **MQTT** connection in the main window connection tree and select `MQTT raw explorer`. The broker, credentials and TLS settings of that connection are used.
- Right-click on the **Raw MQTT Connections** item in the connection tree and select `Add new raw MQTT connection...`. A raw MQTT connection holds broker URL, port, username/password, keepalive, TLS/SSL settings and lists of topics to subscribe and publish to. The `Test...` button checks that the broker can be reached.
- Select `MQTT raw explorer window...` in the main window **File** menu or on the toolbar.

## Connecting

Press `Connect` in the `Connection` group (or on the toolbar). The button shows `Connecting...` while the connection is set up and `Disconnect` when connected. The status label shows the result, for example `Connected to host:port` or the reason a connection failed.

- **Pause/Resume** - Temporarily stop adding incoming messages to the display. The connection is kept open.
- **Receive mode**
  - `Append all` - Every message is added under its topic (the last 250 messages per topic are kept).
  - `Replace per topic` - Only the latest message is kept for each topic. This gives a "current state" view of the broker.

## Subscribing

When the connection is established, all subscribe topics defined in the connection settings are subscribed to automatically (QoS 0).

Enter a topic in the `Subscribe to topics` group, select QoS and press `Subscribe`. MQTT wildcards `+` (one level) and `#` (all remaining levels) can be used. `Unsubscribe` removes the subscription for the entered topic.

The **Subscribe** menu has the same functions, plus:

- `Subscribe configured topics` - Subscribe again to all topics defined in the connection settings.
- `Clear subscriptions` - Clear the window's list of tracked subscriptions (no unsubscribe is sent to the broker).

## Topic tree and message details

Received messages are shown in a tree built from the topic levels. A topic node shows the last payload as a preview and the number of topics/messages below it. Expand a topic to see the individual messages.

Select a message to see its details in the details pane:

| Field | Description |
| ----- | ----------- |
| Topic | Full topic of the message. |
| Timestamp | When the message was received. |
| Format | Detected payload format, `JSON`, `XML`, `Text` or `Binary`. |
| QoS | QoS of the message. |
| Retained | `yes` if this is a retained message. |
| Bytes | Payload size. |
| Message id | MQTT message id (if any). |
| Payload | Decoded payload. JSON is shown as an expandable, colorized tree and XML is pretty printed. |
| Raw payload | The payload exactly as received. |

Selecting a topic node instead shows a summary of the branch and the last payload.

Payloads larger than 256 KiB are not decoded (shown as `Payload skipped for performance`) to keep the window responsive.

## Searching and filtering

- **Topic search** - Show only topics matching a pattern. `*` can be used as wildcard, for example `vscp/*/temperature`.
- **Filter** - A comma separated list of conditions that all must match
  - Plain text - Matches messages containing the text (topic, payload, format etc).
  - `qos=n` - Matches messages with QoS `n`.
  - `retain=true` / `retain=false` - Matches retained/not retained messages.
  - `topic=pattern` - Matches the topic against a pattern (`*` wildcard, the pattern may be quoted).

  Example: `topic=vscp/*, retain=false, temperature`

## Saving data

Press `Save selected` to write data to text files. Two save dialogs are shown in turn:

1. The first saves all messages that are currently visible in the tree (that is, after topic search and filter are applied).
2. The second saves the details of the selected message or topic node. For a message the file contains topic, timestamp, format, QoS, retain flag, size, message id, decoded payload and raw payload.

Cancel a dialog to skip that file.

## Publishing

Use the `Publish` group to send a message:

- **Topic** - Topic to publish on.
- **QoS** - QoS for the message.
- **Retain** - Publish as a retained message.
- **Payload** - The payload to send.

Press `Publish` to send the message.

### Publish topic list

Topics you publish to often can be kept in the publish topic list.

- `Add topic` - Add a new topic to the list.
- `Use selected` - Copy the selected list entry to the topic field. Double-clicking an entry does the same.
- `Clear` - Clear the list.
- `Save` / `Load` - Save the list to, or load it from, a JSON or text file.
