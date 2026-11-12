#!/bin/sh
#
# Add VSCP Works to the desktop menu of the current user (GNOME, KDE, ...)
# without a system install. Run from the source tree or an unpacked package.
#
# Usage: install.sh [path-to-vscp-works-qt]
#

set -e

APPDIR="$(dirname -- "$(readlink -f -- "${0}")")"
DATA="${XDG_DATA_HOME:-$HOME/.local/share}"

# The program: given path, next to this script, in ./bin, in ./build or on the PATH
EXE="$1"
if [ -z "$EXE" ]; then
  for candidate in "$APPDIR/vscp-works-qt" "$APPDIR/bin/vscp-works-qt" "$APPDIR/build/vscp-works-qt"; do
    if [ -x "$candidate" ]; then
      EXE="$candidate"
      break
    fi
  done
fi
if [ -z "$EXE" ]; then
  EXE="$(command -v vscp-works-qt || true)"
fi
if [ -z "$EXE" ] || [ ! -x "$EXE" ]; then
  echo "Can't find the vscp-works-qt program. Give its path as argument." >&2
  exit 1
fi
EXE="$(readlink -f -- "$EXE")"

# Icons in all sizes in the user's hicolor theme
for icon in "$APPDIR"/icons/hicolor/*/apps/vscpworks.png; do
  size="$(basename "$(dirname "$(dirname "$icon")")")"
  mkdir -p "$DATA/icons/hicolor/$size/apps"
  cp "$icon" "$DATA/icons/hicolor/$size/apps/vscpworks.png"
done

# Desktop entry pointing at this program
mkdir -p "$DATA/applications"
sed -e "s|^Exec=.*|Exec=\"$EXE\"|" "$APPDIR/install/vscp-works-qt.desktop" \
  > "$DATA/applications/vscp-works-qt.desktop"
chmod 644 "$DATA/applications/vscp-works-qt.desktop"

# Refresh caches so the menu entry and the icon show up at once
# An existing icon cache must be refreshed or it hides the new icons. Don't
# create one where there is none (a stale cache would hide later icons).
if command -v gtk-update-icon-cache >/dev/null 2>&1 &&
   { [ -f "$DATA/icons/hicolor/icon-theme.cache" ] || [ -f "$DATA/icons/hicolor/index.theme" ]; }; then
  gtk-update-icon-cache -f -t "$DATA/icons/hicolor" >/dev/null 2>&1 || true
fi
if command -v update-desktop-database >/dev/null 2>&1; then
  update-desktop-database "$DATA/applications" >/dev/null 2>&1 || true
fi

echo "Installed VSCP Works ($EXE) in the application menu."
