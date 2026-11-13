# Setting up on Raspberry Pi

This page describes how to set up VSCP Works on a Raspberry Pi. There are no ready-built packages for the Raspberry Pi, so the program is built from source.

VSCP Works needs Qt 6.8.1 or later and CMake 3.31 or later. Raspberry Pi OS based on Debian 13 "trixie" has both (Qt 6.8.2 and CMake 3.31.6). Older versions, based on Debian 12 "bookworm", have Qt 6.4 and CMake 3.25, which are too old. A Raspberry Pi 4 or 5 with at least 4 GB of memory is recommended for building.

## Install the tools and libraries

```bash
sudo apt update
sudo apt install git cmake build-essential \
  qt6-base-dev qt6-declarative-dev qt6-charts-dev qt6-serialbus-dev \
  qt6-serialport-dev qt6-tools-dev \
  libexpat1-dev libssl-dev libsecret-1-dev libmosquitto-dev libcurl4-openssl-dev
```

## Build

```bash
git clone https://github.com/grodansparadis/vscp-works-qt.git
cd vscp-works-qt
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
```

`--parallel 2` limits the number of compilers that run at the same time, so the build does not run out of memory.

Install the program with:

```bash
sudo cmake --install build
```

Or run it directly from the build folder with `./build/vscp-works-qt`.

For CAN interfaces on the Raspberry Pi (for example a CAN HAT), see [SocketCAN](connection_socketcan.md). For more on the Linux build, see [Setting up on Linux](setting_up_the_system_on_linux.md).

---

${/var/creation-time}  
This document is licensed under [Creative Commons BY 4.0](https://creativecommons.org/licenses/by/4.0/) and can be freely copied, redistributed, remixed, transformed and built upon, as long as you give credit to the author.
