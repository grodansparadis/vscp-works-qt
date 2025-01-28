# Building install package on Windows



## Prerequisites

We use Qt Installer Framework to build the install package. You can download the installer from [Qt Installer Framework](https://download.qt.io/official_releases/qt-installer-framework/).

## Build

 * Open a terminal and navigate to the install/windows directory.
 * Create a new folder for the install package.
 * Copy the executable file and the other files in the build release folder to the install folder.
 * Create a folder named "config" in the install folder.
 * Create a file named "config.xml" in the config folder.
 * Copy the following content to the "config.xml" file:

```xml  
<?xml version="1.0" encoding="UTF-8"?>
<Installer>
    <Name>vscp-works-qt</Name>
    <Version>0.0.1</Version>
    <Title>VSCP Works Installer</Title>
    <Publisher>Grodans Paradis AB</Publisher>
    <StartMenuDir>VSCP</StartMenuDir>
    <TargetDir>@HomeDir@/VSCP/VSCP Works Qt</TargetDir>
    <RemoteRepositories>
        <Repository>
            <Url>https://vscp.org/packages</Url>
            <Enabled>true</Enabled>
            <DisplayName>VSCP Project</DisplayName>
        </Repository>
    </RemoteRepositories>
    <ControlScript>installscript.qs</ControlScript>
    <PackageInfo>package.xml</PackageInfo>
    <UserInterfaces>
        <UserInterface>ui/installer.ui</UserInterface>
    </UserInterfaces>
    <InstallerWindowIcon>icon.ico</InstallerWindowIcon> 
</Installer>
```
 * Create a file named "installscript.qs" in the config folder.
 * Copy the following content to the "installscript.qs" file:

```javascript
function Component()
{
    installer.autoRejectMessageBoxes();
    installer.setMessageBoxAutomaticAnswer("OverwriteTargetDirectory", QMessageBox.Yes);
}
```

  * Create a folder named "packages" in the install folder.
  * create a folder org.vscp.vscpworks in the packages folder.
  * Create a folder named "meta" in the org.vscp.vscpworks folder.
  * Create a file named "package.xml" in the meta folder.
  * Copy the following content to the "package.xml" file:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<Package>
    <DisplayName>VSCP Works Qt</DisplayName>
    <Description>The VSCP toolbox</Description>
    <Version>0.0.1</Version>
    <ReleaseDate>2025-01-28</ReleaseDate>
    <Default>true</Default>
    <Script>installscript.qs</Script>
    <UpdateText>Update</UpdateText>
    <Dependencies>
        <Dependency name="vscp-works-qt" version="0.0.1" />
    </Dependencies>
    <Licenses>
        <License nam="MIT" file="license.txt" />
    </Licenses>
</Package>
```
  
  * Put the license file in the meta folder. "license.txt"
  * Create a file named "icon.ico" in the config folder.
  * Copy the icon file to the "icon.ico" file.
  * Create a folder named "ui" in the install folder.
  * Copy the "installer.ui" file to the "ui" folder.
  * Open a terminal and navigate to the Qt Installer Framework bin directory.
  * Run the following command to build the install package: 

```bash
 G:\Qt\QtIFW-4.8.1\bin\binarycreator.exe -c config/config.xml -p packages installer
```


 * Run the following command to build the install package:

```bash
G:\qt\6.8.1\msvc2022_64\bin\windeployqt.exe -c config\config.xml -p packages vscpworks-installer.exe
```




```bash
binarycreator.exe -c config/config.xml -p packages installer
```

## Links
  * Video series on how to use Qt IFW: https://www.youtube.com/watch?v=gnpPosTbttM&list=PLQMs5svASiXNx0UX7tVTncos4j0j9rRa4&index=3
  * Examples: https://doc.qt.io/qtinstallerframework/qtifwexamples.html
  * Documentation: https://doc.qt.io/qtinstallerframework/index.html
  * cpack example for Qt: https://github.com/miurahr/cmake-qt-packaging-example/blob/master/Packaging.cmake
  * cpack example for Qt: https://www.ics.com/blog/revisiting-qt-installer-framework-cmake, https://github.com/christopro/AwesomeWorld-Cmake