# VSCP Works + -- Things to do

c++ pass class member as callback
https://stackoverflow.com/questions/400257/how-can-i-pass-a-class-member-function-as-a-callback

  - All dialogs should use layout for all items

## MQTT
  - Handle v5 properties
  - Handel string, binary, json and xml content
  - First character is not 0x00, "<" or "{" ==> STRING
  - First character 0x00 and content len > 0  ==>  BINARY   (UDP format)
  - First non white space character "<" ==> XML
  - First non white space character "{" ==> JSON

## LOCAL Dialog
  - TODO  Add log file types

### CANAL - dialog


## SOCKETCAN - dialog


## UDP - dialog
  - TODO test connection needs to be implemented.

## MULTICAST - dialog
  - TODO test connection needs to be implemented.

## GUID database
  - TODO GUID to name (database, DNS).
  - TODO Load/save GUID sets

## RAW CANAL
  - TODO Collect id's and allow to filtering on them (SavveyCAN).
  - TODO Two views (ID frequency, messages) a'la PEAK.

## Session window
  - TODO Colormarking from filter. Match set filter/icon.
  - TODO Set filter from recived event.
  - Undefined events should be possible to display info for as user defined evenst                           
  - Received event cloned to transmit
  - Received event cloned to filter and !filter
  - Toolbar: connect | filter active | filter combo | base combo
  - Define your own data rendering (Javascript/mustasch)
  - Custom data rendering for any event from db.
  - Config device from session window.
  - Change line and status info when GUID symbolic or sensor symbolic has been updated
  - MQTT: activate/deactivate subscription on the fly. (publish topics also maybe)
  - Better and more flexible sizing
  - Message receive should be reprogrammed using std::bind and std::function 
     https://stackoverflow.com/questions/400257/how-can-i-pass-a-class-member-function-as-a-callback

## Configuration window
 - search register / next 
  - search remote variable /next
  - Remove ctrl+0/1/2... from menu keep functionality.
  - Change node id after scan does not work
  - From firmware file -> boot of that file.

## Scan window
  - Go to firmware update directly from window
  - Full Level II device scan.
  - Go to bootload

## Bootloader window


## MDF editor window
- No way to add file items
- Initial path is wrong
- Encoding/decoding of strings to match JSON/XML.


## GUID Known nodes Dialog
TODO Dialog with add/edit/delete of known GUID's.   CDlgGuid.ui

## Dialog log viewer
TODO Dialog with log viewing/filtering   CDlgLog

## Misc.
Generate code for web page interfaces

## FUTURE
https://github.com/mguentner/cannelloni
