#!/bin/sh

CURDIR=`pwd`
APPDIR="$(dirname -- "$(readlink -f -- "${0}")" )"

cd "$APPDIR"

# Set absolute path work around
sed -e "s,FULLPATH,$PWD,g" vscp-works-qt.desktop > vscp-works-qt.desktop.temp

cp vscp-works-qt.desktop.temp ~/.local/share/applications/vscp-works-qt.desktop
cp vscp-works-qt.desktop.temp ~/Desktop/vscp-works-qt.desktop
rm vscp-works-qt.desktop.temp

echo "Installed vscp-works-qt icons on menu and desktop !"

