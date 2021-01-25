# vscp-works-qt

## !!!! WARNING !!!!!    Work in progress
This is the project that eventually will be the next version of **vscpworks**. It is a total rewrite and is still far from a usable state. There is no documentation. No user setup information and help.

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
