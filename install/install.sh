#!/bin/sh

CURDIR=`pwd`
APPDIR="$(dirname -- "$(readlink -f -- "${0}")" )"

cd "$APPDIR"

# Set absolute path work around
sed -e "s,FULLPATH,$PWD,g" vscpworks.desktop > vscpworks.desktop.temp

cp vscpworks.desktop.temp ~/.local/share/applications/vscpworks.desktop
cp vscpworks.desktop.temp ~/Desktop/vscpworks.desktop
rm vscpworks.desktop.temp

echo "Installed vscpworks icons on menu and desktop !"

