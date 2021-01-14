# vscp-works-qt

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