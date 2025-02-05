# Setting up on Linux

This document describes how to set up the VSCP Works application on a Linux system. You can either build the executable from source yourself or more convenient download a binary version. Instructions on how to get the binary is [here](./setting_up_the_system.md).

## Build from source

### Prerequisites  
  - You need to have git installed on your system. If not, you can install it by running the following command:
    ```bash
    sudo apt install git
    ```
  - You need to have cmake installed on your system. If not, you can install it by running the following command:
    ```bash
    sudo apt install cmake
    ```
  - You need to have a C++ compiler installed on your system. If not, you can install it by running the following command:
    ```bash
    sudo apt install build-essential
    ```
  - 

### Install Qt.

You need to have Qt installed on your system. If not, you can install it by following the instructions [here](https://www.qt.io/download-open-source). Any version 6.x should work.

### Install other needed libs

You need expat, mosquitto, websockets, openssl and libcurl to build this project

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
### Clone the vscp-works-qt repository in a folder.
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
  make install
```

where **CMAKE_PREFIX_PATH** should point to the Qt version you installed previously

You can use

```bash
  cmake -DCMAKE_BUILD_TYPE=Debug ..
```

if you want to build a version suitable for debugging.

### Run the application
```bash
  vscp-works-qt
```
