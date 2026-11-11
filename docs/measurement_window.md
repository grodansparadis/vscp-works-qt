# Realtime measurement window

The realtime measurement window follows one measurement value from a session window, as a diagram, as a plain value, as a gauge or as a percentage bar. Several measurement windows can be open at the same time, for example one for each sensor you are interested in.

## Opening the window

1. Open a [session window](session_window.md) and connect.
2. Select a row in the receive list that holds a VSCP **measurement** event (for example `CLASS1.MEASUREMENT`).
3. Right-click and select `Open realtime measurement window...`, or select `Tools/Open realtime measurement window...` in the session window menu.

The measurement source is defined by the selected event: **VSCP class**, **VSCP type**, **sensor index**, **unit** and **GUID**. From now on every event received in the session window that matches all of these is added to the measurement window. The source is shown at the top of the window, for example `Class=10, Type=6, Sensor=0, Unit=Celsius (C), GUID=...`.

The window is closed automatically if the session window it belongs to is closed.

## Display modes

Choose how the value is presented with the `Display mode` combo box.

### Diagram

A line chart with the measurement history. The `Diagram axis` combo box selects what is shown on the x-axis:

| Axis mode | Description |
| --------- | ----------- |
| Realtime | Seconds since the first sample in the history. |
| Timestamp | Absolute date/time of each sample. |
| HH:MM:SS | Time of day of each sample. |

**Zooming** - Drag a rectangle with the mouse in the chart to zoom into that area. Use the `+` and `-` buttons to zoom in/out and `Reset` to show the whole history again.

### Value

The latest value shown in large digits together with minimum, maximum and average of the history.

### Gauge

A round gauge with the latest value. The gauge range is the lowest and highest value seen. Three gauge styles are available in the `Gauge style` combo: `Classic`, `Blue needle` and `Accent ring`.

### Percent

A bar showing the value as a percentage (0-100). Values below 0 or above 100 are clipped, the raw value is shown below the bar. Suitable for measurements such as relative humidity or levels that already are in percent.

## Unit and statistics

The unit of the measurement is looked up in the VSCP event database and shown as `Unit: name (symbol)`. Diagram and value modes also show `Min`, `Max` and `Avg` for the samples in the history.

The window keeps the last 500 samples.

## Alarm

Check `Alarm: Enable`, enter a threshold value and select `Above` or `Below`. The computer beeps when a new value crosses the threshold (value above respectively below the threshold). A new beep is not given until the value has returned to the normal side of the threshold and crossed it again.

## Pause, clear and save

- **Pause/Resume** - Freeze the display. Values received while paused are still recorded (and the alarm is still evaluated), and they are shown when you press `Resume`.
- **Clear** - Remove all samples from the history.
- **Save...** - Save the samples to a CSV file. The file has the columns `timestamp_utc` (ISO 8601 with milliseconds) and `value`.
