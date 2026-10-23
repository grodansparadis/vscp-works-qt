# Install files

## Files

* `AppRun` - The script that starts the application
* `vscp-works-qt.desktop` - The desktop file
* `vscpworks.png` - The icon file
* `vscpworks_128x128.png` - The icon file
* vscpworks-version.json - The version file that sits on vscp.org/version/vscpworks-qt.json and can be used to check what the latest version of vscp works is.

## Application icons

The application uses [icons/vscpworks.png](../icons/vscpworks.png) as its
Qt window icon on all platforms.

* Linux: CMake installs the desktop entry and the icon in
  `share/icons/hicolor/128x128/apps/vscpworks.png`. The desktop file name is
  also set in Qt so desktop shells can associate running windows with the launcher.
  Linux uses this desktop integration rather than an icon embedded in the ELF binary.
* Windows: [src/vscpworks.rc](../src/vscpworks.rc) embeds
  [icons/vscpworks.ico](../icons/vscpworks.ico) in the executable.
* macOS: CMake builds and installs an application bundle with
  [icons/vscpworks.icns](../icons/vscpworks.icns) in `Contents/Resources`
  and sets `CFBundleIconFile` through `MACOSX_BUNDLE_ICON_FILE`.

The legacy qmake project also selects the Windows and macOS native icons.
The ICO and ICNS assets are generated from the PNG; regenerate both when
changing the artwork. They are checked in, so building does not require image
conversion tools. With ImageMagick and Python Pillow installed:

```sh
magick icons/vscpworks.png -define icon:auto-resize=128,64,48,32,16 icons/vscpworks.ico
python3 -c 'from PIL import Image; Image.open("icons/vscpworks.png").save("icons/vscpworks.icns")'
```
