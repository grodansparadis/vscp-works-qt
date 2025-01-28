# Building install package on Linux

The installation package for Linux is built on a Ubuntu 20.04.2 LTS system (oldest supported LTS) using linuxdeployqt. 

The _build-inst-cmake.sh_ script is used to build the installation package. The script is located in the `install` folder.

The installation package is built using the following steps:

1. Created build folder and change to it
```bash
mkdir build
cd build
```

2. Build the project

```bash
# configure build files with CMake
# we need to explicitly set the install prefix, as CMake's default is /usr/local for some reason...
cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release 
# build project and install files into AppDir
make -j$(nproc)
make install DESTDIR=AppDir
```
3. Copy files to AppDir that cmake does not copy

```bash
# Prepare content not installed by cmake
cp ../install/AppRun AppDir
chmod a+x AppDir/AppRun
cp ../install/vscp-works-qt.desktop AppDir
cp ../icons/vscpworks.png AppDir
mkdir -p AppDir/usr/share/applications/
cp ../install/vscp-works-qt.desktop AppDir/usr/share/applications/
mkdir -p AppDir/usr/share/icons/hicolor/128x128/apps/
cp ../icons/vscpworks_128x128.png AppDir/usr/share/icons/hicolor/128x128/apps/vscpworks.png 
``` 


4. Install linuxdeployqt

Install the linuxdeployqt tool. The tool is used to create the installation package. Make the file executable.

```bash
wget -c https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage
chmod a+x linuxdeployqt-continuous-x86_64.AppImage
```

5. Set the path to the Qt installation

```bash
export PATH=~/Qt/6.8.1/gcc_64/bin/:$PATH
``` 

6. Run linuxdeployqt

```bash./vs 
# Run linuxdeployqt
ARCH=x86_64 ./linuxdeployqt-continuous-x86_64.AppImage AppDir/usr/share/applications/vscp-works-qt.desktop -exclude-libs=libqsqlmimer -appimage
```

The libsqlmimer file is commercial and not allowed to be distributed. It is excluded from the installation package. libodbc1 or libodbc2 needs to be installed and libpq-dev for PostgreSQL support. This is needed for the database plugin to work even if the specific databases is not used.

6. The installation package is now created and can be found in the `build` folder.

It is now possible to to use the Qt installation framework to create an installer for the installation package. But the AppImage will suffice for most cases.

7. You can also build a tree with files and use appimage to make an AppImage file. See the link below for more information.

```bash
./appimagetool-i686.AppImage AppDir
```

# Reference material

 * [Deploying Qt Applications (windows, Mac, Linux)](https://www.youtube.com/playlist?list=PLQMs5svASiXNx0UX7tVTncos4j0j9rRa4)
 * [Qt Installer Framework](https://doc.qt.io/qtinstallerframework/ifw-tutorial.html)
 * [linuxdeployqt](https://github.com/probonopd/linuxdeployqt/releases)
 * [AppImage](https://github.com/AppImage/AppImageKit)