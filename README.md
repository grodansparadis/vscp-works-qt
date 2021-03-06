# vscp-works-qt

## !!!!! WARNING !!!!!!!  This IS Work in progress
This is the project that eventually will be the next version of **vscpworks**. It is a total rewrite and is still far far far from a usable state. There is no documentation. No user setup information and no help.

## Build
If you still want to build this project

### Install qt 5.0.
```bash
  sudo apt install qttools5-dev
  sudo apt install qt5-default
  sudo apt install libqt5serialbus5-dev
  sudo apt install qtdeclarative5-dev
  sudo apt install libqt5serialport5
  sudo apt install libqt5serialport5-dev
```

### Install other needed libs

You need expat, mosquitto, openssl to build this project

```bash
  sudo apt install libexpat-dev
  sudo apt install libssl-dev
  sudo apt install libpaho-mqtt1.3  
```

Paho version 1.3 or bigger should be used. Build manually from [Paho code repository](https://github.com/eclipse/paho.mqtt.c) with

```bash
  cmake .. -DPAHO_WITH_SSL=TRUE
```

to enable needed SSL



### Clone this repository in a folder.
```bash
  git clone --recurse-submodules -j8 https://github.com/grodansparadis/vscp-works-qt.git
```  

### Temporary step while development branch of VSCP is used
```
cd vscp-works-qt
cd vscp
git pull origin development
cd ..
```

May need

```
git pull origin development --allow-unrelated-histories 
```

### Go to the build folder and make the project
```bash
  cd vscp-works-qt
  mkdir build
  cd build
  cmake ..
```

use

```bash
  cmake -DCMAKE_BUILD_TYPE=Debug ..
```

if you want to build a version suitable for debugging.

----

**Internal project Notes below**

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


## cmake

### Problems

CMake Error at /usr/lib/x86_64-linux-gnu/cmake/Qt5/Qt5Config.cmake:28 (find_package):
  Could not find a package configuration file provided by "Qt5LinguistTools"
  with any of the following names:

    Qt5LinguistToolsConfig.cmake
    qt5linguisttools-config.cmake


Install qttools5-dev, qt5-default and qtdeclarative5-dev

git submodule update --init --recursive

git pull --recurse-submodules

git submodule foreach git pull origin master
s



git clone --recurse-submodules -j8 https://github.com/grodansparadis/vscp-works-qt.git

cd vscp-works-qt
cd src
mkdir build
cd build
cmake ..


----

## Linux

  * Need to install libqt5serialport5-dev

  sudo apt-get install libqt5serialport5
  sudo apt-get install libqt5serialport5-dev

## Enable debug

  cmake -DCMAKE_BUILD_TYPE=Debug

## deb install

  * https://www.debian.org/doc/manuals/debmake-doc/ch08.en.html#cmake-single
