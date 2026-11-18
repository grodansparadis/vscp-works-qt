#!/usr/bin/env bash

set -euo pipefail

version_file="${GITHUB_WORKSPACE:-$(pwd)}/src/version.h"

read_component() {
  local name="$1"
  sed -nE "s/^#define ${name}[[:space:]]+([0-9]+).*$/\\1/p" "$version_file"
}

major=$(read_component VSCPWORKS_MAJOR_VERSION)
minor=$(read_component VSCPWORKS_MINOR_VERSION)
patch=$(read_component VSCPWORKS_PATCH_VERSION)

if [[ -z "$major" || -z "$minor" || -z "$patch" ]]; then
  echo "Could not read the VSCP Works version from $version_file" >&2
  exit 1
fi

version="$major.$minor.$patch"
if [[ -n "${GITHUB_OUTPUT:-}" ]]; then
  echo "version=$version" >>"$GITHUB_OUTPUT"
fi
echo "$version"
