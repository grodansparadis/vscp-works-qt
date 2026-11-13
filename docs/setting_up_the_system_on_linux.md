# Setting up on Linux

This page describes how to set up VSCP Works on a Linux system. You can build the program from source yourself or, more conveniently, download a binary version (an AppImage for x86-64). Instructions for getting the binary are [here](./setting_up_the_system.md).

## Build from source

### Requirements

- Qt 6.8.1 or later with the `Qt Charts`, `Qt Serial Bus`, `Qt Serial Port` and `Qt Help` modules (and Qt Quick/QML, which are part of the standard Qt installation).
- CMake 3.31 or later.
- A C++ compiler, Git and Python 3 (used by the build tests).
- The libraries expat, mosquitto, OpenSSL, libcurl and libsecret (libsecret lets VSCP Works keep passwords and keys in GNOME Keyring and other Secret Service keyrings).

Debian 13 "trixie" (and distributions based on it) has all of this in its package repositories. Older distributions, for example Ubuntu 24.04, have an older Qt and CMake; install Qt with the [Qt online installer](https://www.qt.io/download-qt-installer) (select the modules above) and a newer CMake, for example from [cmake.org](https://cmake.org/download/) or with `pip install cmake`.

### Install the tools and libraries

```bash
sudo apt install git cmake build-essential python3 \
  libexpat1-dev libssl-dev libsecret-1-dev libmosquitto-dev libcurl4-openssl-dev
```

If you use the Qt packages of the distribution (Debian 13 and later):

```bash
sudo apt install qt6-base-dev qt6-declarative-dev qt6-charts-dev \
  qt6-serialbus-dev qt6-serialport-dev qt6-tools-dev
```

If you use Qt from the online installer, you may also need the OpenGL development files:

```bash
sudo apt install libglx-dev libgl1-mesa-dev
```

### Clone the vscp-works-qt repository

```bash
git clone https://github.com/grodansparadis/vscp-works-qt.git
```

All third party dependencies that are built from source (the VSCP main
repository among them) are downloaded automatically by CMake (FetchContent)
during the configure step, so no git submodules are needed.

### Build the project

```bash
cd vscp-works-qt
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
sudo cmake --install build
```

If Qt comes from the online installer, tell CMake where it is, for example:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=~/Qt/6.11.1/gcc_64
```

Use `-DCMAKE_BUILD_TYPE=Debug` to build a version for debugging.

### Make a package

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DVSCPWORKS_DEPLOY_RUNTIME=ON
cmake --build build --parallel
cd build
cpack
```

builds a `.tar.gz` archive that contains the Qt libraries and plugins, so it runs on systems without Qt (or with another Qt version). The system must still provide the basic libraries (glibc, X11/xcb including `libxcb-cursor0`, OpenGL, fontconfig) and mosquitto, expat, OpenSSL and libcurl. Without `VSCPWORKS_DEPLOY_RUNTIME` the archive and `cmake --install` use the Qt installed on the system.

The AppImage built by the project's GitHub workflow bundles everything except the basic system libraries and runs on distributions with glibc 2.35 or newer (Ubuntu 22.04, Debian 12 and later).

### Run the application

```bash
vscp-works-qt
```

---

${/var/creation-time}  
This document is licensed under [Creative Commons BY 4.0](https://creativecommons.org/licenses/by/4.0/) and can be freely copied, redistributed, remixed, transformed and built upon, as long as you give credit to the author.
