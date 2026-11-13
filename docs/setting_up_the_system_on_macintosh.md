# Setting up on macOS

This page describes how to set up VSCP Works on macOS. You can download a ready-built application or build it from source.

## Download

Releases on the [VSCP Works GitHub Releases](https://github.com/grodansparadis/vscp-works-qt/releases) page include a macOS package (`vscp-works-qt-macos-install_<version>.zip`). Unpack it and move `vscp-works-qt.app` to the `Applications` folder. The Qt frameworks and other libraries the program needs are included in the application bundle.

The application is not signed. The first time you start it, right-click (or Ctrl-click) it in Finder and select `Open`, then confirm that you want to open it.

## Build from source

You need the Xcode command line tools, [Homebrew](https://brew.sh) and Qt 6.8.1 or later with the `Qt Charts`, `Qt Connectivity`, `Qt Serial Bus`, `Qt Serial Port` and `Qt WebSockets` modules (install them with the [Qt online installer](https://www.qt.io/download-qt-installer)). CMake 3.31 or later is required.

Install the tools and libraries:

```bash
xcode-select --install
brew install cmake ninja expat mosquitto openssl@3
```

Get the source and build it. Change the Qt path to where Qt is installed:

```bash
git clone https://github.com/grodansparadis/vscp-works-qt.git
cd vscp-works-qt
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=~/Qt/6.11.1/macos \
  -DOPENSSL_ROOT_DIR=$(brew --prefix openssl@3)
cmake --build build --parallel
```

To make an application bundle that includes Qt and the Homebrew libraries, install it:

```bash
cmake --install build --prefix ~/vscp-works
```

`~/vscp-works/vscp-works-qt.app` can then be moved to the `Applications` folder.

Passwords and keys are stored in the macOS Keychain, see [Passwords and keys](passwords_and_keys.md).

---

${/var/creation-time}  
This document is licensed under [Creative Commons BY 4.0](https://creativecommons.org/licenses/by/4.0/) and can be freely copied, redistributed, remixed, transformed and built upon, as long as you give credit to the author.
