# vscp-works-qt


![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Linux Build](https://github.com/grodansparadis/vscp-works-qt/actions/workflows/cmake.yml/badge.svg) 
![Windows Build](https://github.com/grodansparadis/vscp-works-qt/actions/workflows/msbuild.yml/badge.svg)
[![Project Status: Active – The project has reached a stable, usable state and is being actively developed.](https://www.repostatus.org/badges/latest/wip.svg)](https://www.repostatus.org/#wip)


This is the new version of **vscpworks** that replaces [the long since deprecated version](https://github.com/grodansparadis/vscpworks). It is a total rewrite and there is still a lot of things to add and fix. There is very little documentation but it will grow as we go. The state of the code should be regarded as alpha still for some time.  

That said a lot of functionality works and it may be a useful tool for VSCP developers. 

  * Communication channels can be opened over socketcan/CANAL/tcp/ip and MQTT to remote devices. GUID's and sensors can have symbolic names. Easy to investigate event values and timing.  
  * Configuration handling can be opended to remote devices over socketcan/CANAL/tcp/ip and MQTT.
  * Scan of devices can be handled for remote devices over socketcan/CANAL/tcp/ip and MQTT.
  * Firmware loading can be done using the VSCP bootloader protocol and the PIC1 protocol. 
  * mdf editor is available that can be used to build mdf-files in XML or json format. This solution may still have problems with some more advanced files so be careful and save often.

Binary versions for Linux and windows is available in the release section.

The manual is available [here]()

## Build on Linux
If you want to build this project on linux follow the steps below

### Install Qt.

Go to the qt site and download qt using the [Qt Online Installer](https://www.qt.io/download-open-source). Any version 6.x should work.

### Install other needed libs

You need expat, mosquitto, openssl and libcurl to build this project

```bash
  sudo apt install libexpat-dev
  sudo apt install libssl-dev
  sudo apt install libmosquitto-dev
  sudo apt install libwebsockets-dev
  sudo apt-get install libcurl4-openssl-dev
  #sudo apt install libspdlog-dev
  # For Qt OpenGL support
  sudo apt install libglx-dev libgl1-mesa-dev
  # For Qt database support
  sudo apt install libodbc1 or libodbc2
  sudo apt install libpq-dev

```
### Clone the vscp-works-qt repository in a folder (on the same level as vscp above).
```bash
  git clone  --recurse-submodules -j4 https://github.com/grodansparadis/vscp-works-qt.git
```  

### Go to the build folder and make the project
```bash
  cd vscp-works-qt
  mkdir build
  cd build
  cmake -DCMAKE_PREFIX_PATH=~/Qt/6.8.1/gcc_64 ..
  make  
```

where **CMAKE_PREFIX_PATH** should point to the Qt version you installed previously

You can use

```bash
  cmake -DCMAKE_BUILD_TYPE=Debug ..
```

if you want to build a version suitable for debugging.

## Build on Windows

If you don't have Visual Studio Code installed it is recommended. You can find it [here](https://code.visualstudio.com/download).

Install cmake-tools, c++ extension

```bash
ext install cmake-tools
```

### VSCP main and VSCP works repositories
You need to checkout the VSCP main repository code as well (but not in the vcpkg folder). You do this with

```bash
  git clone --recurse-submodules -j8 https://github.com/grodansparadis/vscp.git
  cd vscp
  git checkout development
``` 

and the vscp-works-qt code

```bash
git clone --recurse-submodules -j8 https://github.com/grodansparadis/vscp-works-qt.git
```

### Optionally install Qt for uae of Qt tools

Go to the [qt site](https://www.qt.io/download) and follow the instructions

### Install the vcpkg package manager

Install the vcpkg package manager by cloning its github repository in a folder

```bash
git clone https://github.com/microsoft/vcpkg.git
```

then go into the folder

```bash
cd vcpkg
```

Run the vcpkg bootstrapper command

```bash
bootstrap-vcpkg.bat
```

The process is described in detail [here](https://docs.microsoft.com/en-us/cpp/build/install-vcpkg?view=msvc-160&tabs=windows)

To [integrate with Visual Studio](https://docs.microsoft.com/en-us/cpp/build/integrate-vcpkg?view=msvc-160) run

```bash
vcpkg integrate install
```

Install the required libs

```bash
vcpkg install pthread:x64-windows
vcpkg install dlfcn-win32:x64-windows
vcpkg install expat:x64-windows
vcpkg install openssl:x64-windows
vcpkg install paho-mqtt:x64-windows
```

The Qt installation may take some time

```bash
vcpkg list
```

will give 

![](images/vcpkg-install-list.png)

after installing the required libs

Full usage is describe [here](https://docs.microsoft.com/en-us/cpp/build/manage-libraries-with-vcpkg?view=msvc-160&tabs=windows)


Build as usual but use

```bash
cd vscp-vscp-works
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DVCPKG_ROOT=G:/vcpkg/ -DCMAKE_TOOLCHAIN_FILE=G:/vcpkg/scripts/buildsystems/vcpkg.cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=g:/Qt/6.8.1/msvc2022_64
```

The **VCPKG_ROOT** and **CMAKE_TOOLCHAIN_FILE** path is most certainly different in your case

Note that *Release* should be either *Release* or *Debug* as of your preferences

_"Visual Studio 17 2022"_ may be _"Visual Studio 16 2019"_ or some other value depending on what Visual Studio you have installed

The windows build files can now be found in the build folder and all needed files to run the project can  after build - be found in build/release or build/Debug depending on CMAKE_BUILD_TYPE setting.

Building and configuration is simplified with VS Code installed. Configure/build/run can be done (see lower toolbar). Using VS Code it may be useful to add

```json
"cmake.configureSettings": {
   "CMAKE_BUILD_TYPE": "${buildType}"
}
``` 

to your settings.json file. Or to use 

To build at the command prompt use **--config Release** or **--config Release**

```bash
cmake --build . --config Release
```

or 

```bash
msbuild vscp-works-qt.sln
```

Note that you must have a *developer command prompt*

**Note:** You may experience a lot of warnings from spdlog which is a library that is part if vscp-works-qt. This is nothing to to worry about. More info on the subject is [here](https://github.com/gabime/spdlog/issues/2065).

-------------------------------------------------------------------------------

**Internal project Notes and trouble shooting below**

## config

The config file for VSCP works qt is JSON based and have the following format

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "test session",
            "type": "0"
        }
    ]
}
```

### configuration
Each item in the configuration section is a connection to a remote host. All have "name" and "type" in common while other tags may depend for different types.

#### name 
The name that is displayed to the user

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

##### No Connection

----

## Linux

## Enable debug

  cmake -DCMAKE_BUILD_TYPE=Debug

## deb install

  * https://www.debian.org/doc/manuals/debmake-doc/ch08.en.html#cmake-single
