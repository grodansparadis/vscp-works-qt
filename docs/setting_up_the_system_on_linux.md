# Setting up on Linux

This page describes how to set up VSCP Works on a Linux system. You can build the program from source yourself or, more conveniently, download a binary version. Instructions for getting the binary are [here](./setting_up_the_system.md).

## Build from source

### Prerequisites  
  - Git must be installed. If it is not, install it with:
    ```bash
    sudo apt install git
    ```
  - CMake must be installed. If it is not, install it with:
    ```bash
    sudo apt install cmake
    ```
  - A C++ compiler must be installed. If it is not, install it with:
    ```bash
    sudo apt install build-essential
    ```

### Install Qt

Qt must be installed. If it is not, follow the instructions [here](https://www.qt.io/download-open-source). Any 6.x version should work.

### Install other required libraries

You need expat, mosquitto, libwebsockets, OpenSSL and libcurl to build the project:

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
### Clone the vscp-works-qt repository
```bash
  git clone  --recurse-submodules -j4 https://github.com/grodansparadis/vscp-works-qt.git
```  

### Build the project
```bash
  cd vscp-works-qt
  mkdir build
  cd build
  cmake -DCMAKE_PREFIX_PATH=~/Qt/6.11.1/gcc_64 ..
  make
  make install
```

where **CMAKE_PREFIX_PATH** points to the Qt version you installed earlier.

Use

```bash
  cmake -DCMAKE_BUILD_TYPE=Debug ..
```

if you want to build a version for debugging.

### Run the application
```bash
  vscp-works-qt
```
