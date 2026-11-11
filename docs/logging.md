# Logging

VSCP Works + can log what it does to a log file and to the console. Logs are the first place to look when something does not work as expected, for example when a connection fails or an MDF can not be parsed. Most error dialogs in the program ask you to *"see log for more info"*.

## Log settings

Logging is configured on the `Logging` tab of the [settings dialog](settings.md) (`Edit/Settings...` in the main window).

| Setting | Description |
| ------- | ----------- |
| File log level | Lowest severity written to the log file. One of `No logging`, `Critical`, `Errors`, `Warning`, `Information`, `Debug` or `Trace`. |
| File log pattern | Format of each log line ([spdlog pattern](https://github.com/gabime/spdlog/wiki/3.-Custom-formatting)). Default is `[vscpworks+] [%^%l%$] %v`. |
| File log max size | Maximum size in bytes of a log file before it is rotated. Default is 5242880 (5 MB). |
| File max logs | Number of rotated log files to keep. Default is 7. |
| File log path | Path to the log file. Use `Set file log path` to select it. |
| Console log level | Lowest severity written to the console (the terminal the program is started from). |
| Console log pattern | Format of each console log line. |

The default log file is `vscpworks.log` in the `logs` folder of the program data folder. See [files and directory structure](files_and_directory_structure.md).

Set the level to `Debug` or `Trace` when you are troubleshooting. Remember to set it back afterwards, as these levels generate a lot of data.

## Log viewer

Select `Help/View Log...` in the main window to open the application log viewer. It reads the log file configured in the settings and shows one row per log entry with its **level** and **message**. Entries are colored by level.

| Control | Description |
| ------- | ----------- |
| Level | Show entries with this severity **or higher**. `All` shows everything. For example `Warning` shows warnings, errors and critical entries. |
| Search | Show only entries where the message (or level) contains the text. Case insensitive. |
| ⟳ Refresh | Reload the log file. |
| Auto-refresh (5s) | Reload the log file every five seconds. Useful to follow the log while you work in other windows. |
| Clear Display | Clear the displayed entries. The log file itself is **not** deleted. |
| Save... | Save the visible (filtered) entries to a text file. |

The number of visible entries is shown at the bottom of the window (`Entries: n`).

If the log file does not exist (for example because file logging is turned off) the viewer shows a single entry telling which file it tried to open.
