#!/usr/bin/env bash
#
# write-sha256sums.sh
#
# Usage: write-sha256sums.sh [NAME] <patterns
#
# Used by the CI workflows before each actions/upload-artifact step. Reads that
# step's 'path' patterns on stdin (one per line, '!' marks an exclude), writes
# the SHA-256 checksums of the matched files to NAME (default SHA256SUMS) in
# the directory that upload-artifact uses as the artifact root, and sets the
# step output 'file' to that file's path relative to the workspace. Add the
# output to the upload's 'path' so the checksums are uploaded with the files.
# The workflows use NAME=SHA256SUMS-<artifact name>, so files from several
# artifacts don't clash when a release job collects them all. Check a
# downloaded artifact with "sha256sum -c SHA256SUMS-<artifact name>".
#

# Needs bash 4 or later. macOS ships bash 3.2, so use Homebrew's bash there.
if [ "${BASH_VERSINFO[0]:-0}" -lt 4 ]; then
  for b in /opt/homebrew/bin/bash /usr/local/bin/bash; do
    if [ -x "$b" ] && [ "$("$b" -c 'echo ${BASH_VERSINFO[0]}')" -ge 4 ]; then
      exec "$b" "$0" "$@"
    fi
  done
  echo "$(basename "$0") needs bash 4 or later (on macOS: brew install bash)" >&2
  exit 1
fi

set -euo pipefail
shopt -s globstar nullglob dotglob

name="${1:-SHA256SUMS}"
name="${name//[^A-Za-z0-9._-]/-}"
out="${GITHUB_OUTPUT:-/dev/stdout}"
cd "${GITHUB_WORKSPACE:-.}"
ws="${GITHUB_WORKSPACE:-$(pwd)}"
ws="${ws//\\//}"
ws="${ws%/}"

trim() {
  local s="$1"
  s="${s#"${s%%[![:space:]]*}"}"
  s="${s%"${s##*[![:space:]]}"}"
  printf '%s' "$s"
}

# Make a pattern relative to the workspace when it points inside it. The
# comparison ignores case so Windows drive letters (D:/ vs d:/) match.
relative() {
  local p="${1//\\//}"
  p="${p#./}"
  local lp="${p,,}" lws="${ws,,}"
  if [ "$lp" = "$lws" ]; then
    p="."
  elif [ "${lp:0:${#lws}+1}" = "$lws/" ]; then
    p="${p:${#ws}+1}"
  fi
  if [ "$p" != "/" ]; then
    p="${p%/}"
  fi
  printf '%s' "$p"
}

# Literal part of a pattern up to the first path segment with a wildcard
search_root() {
  local p="$1" root="" seg
  IFS='/' read -r -a segs <<<"$p"
  for seg in "${segs[@]}"; do
    case "$seg" in
      *[\*\?\[]*) break ;;
    esac
    if [ -z "$root" ]; then
      if [ -z "$seg" ]; then root="/"; else root="$seg"; fi
    elif [ "$root" = "/" ]; then
      root="/$seg"
    else
      root="$root/$seg"
    fi
  done
  printf '%s' "${root:-.}"
}

# Expand a pattern to files (directories expand to all files below them)
expand() {
  local p="$1" m
  local IFS=$'\n'
  # shellcheck disable=SC2206
  local matches=($p)
  if [ ${#matches[@]} -eq 0 ] && [ -e "$p" ]; then
    matches=("$p")
  fi
  for m in "${matches[@]}"; do
    if [ -d "$m" ]; then
      find "$m" -type f
    elif [ -f "$m" ]; then
      printf '%s\n' "$m"
    fi
  done
}

includes=()
excludes=()
while IFS= read -r line || [ -n "$line" ]; do
  line=$(trim "$line")
  [ -z "$line" ] && continue
  case "$line" in \#*) continue ;; esac
  if [ "${line:0:1}" = "!" ]; then
    excludes+=("$(relative "${line:1}")")
  else
    includes+=("$(relative "$line")")
  fi
done

declare -A excluded=()
for p in "${excludes[@]+"${excludes[@]}"}"; do
  while IFS= read -r f; do
    [ -n "$f" ] && excluded["${f#./}"]=1
  done < <(expand "$p")
done

declare -A seen=()
files=()
for p in "${includes[@]+"${includes[@]}"}"; do
  while IFS= read -r f; do
    [ -z "$f" ] && continue
    f="${f#./}"
    case "$(basename "$f")" in SHA256SUMS*) continue ;; esac
    [ -n "${excluded[$f]+x}" ] && continue
    skip=0
    for e in "${!excluded[@]}"; do
      case "$f" in "$e"/*) skip=1; break ;; esac
    done
    [ "$skip" = 1 ] && continue
    [ -n "${seen[$f]+x}" ] && continue
    seen["$f"]=1
    files+=("$f")
  done < <(expand "$p")
done

if [ ${#files[@]} -eq 0 ]; then
  echo "write-sha256sums: no files matched, nothing to do"
  echo "file=" >>"$out"
  exit 0
fi

# Artifact root as upload-artifact computes it: the search paths of the
# patterns, without paths below another one; a single file uses its directory,
# several search paths use their common ancestor.
roots=()
for p in "${includes[@]}"; do
  roots+=("$(search_root "$p")")
done
mapfile -t roots < <(printf '%s\n' "${roots[@]}" | sort -u)
kept=()
for r in "${roots[@]}"; do
  inside=0
  for o in "${roots[@]}"; do
    [ "$o" = "$r" ] && continue
    if [ "$o" = "." ] || [ "${r#"$o"/}" != "$r" ]; then inside=1; break; fi
  done
  [ "$inside" = 0 ] && kept+=("$r")
done

if [ ${#kept[@]} -eq 1 ]; then
  root="${kept[0]}"
  if [ -f "$root" ]; then
    root=$(dirname "$root")
  fi
else
  root="${kept[0]}"
  for r in "${kept[@]:1}"; do
    while [ "$root" != "." ] && [ "$root" != "/" ] && [ "${r#"$root"/}" = "$r" ] && [ "$r" != "$root" ]; do
      root=$(dirname "$root")
    done
  done
fi

rel=()
for f in "${files[@]}"; do
  if [ "$root" = "." ]; then
    rel+=("$f")
  else
    rel+=("${f#"$root"/}")
  fi
done

tmp=$(mktemp)
if command -v sha256sum >/dev/null; then
  (cd "$root" && sha256sum -b "${rel[@]}") >"$tmp"
else
  (cd "$root" && shasum -a 256 -b "${rel[@]}") >"$tmp"
fi
# Build output written from inside a Docker container can be owned by root
if [ -w "$root" ]; then
  cp "$tmp" "$root/$name"
else
  sudo cp "$tmp" "$root/$name"
fi
rm -f "$tmp"
cat "$root/$name"

if [ "$root" = "." ]; then
  echo "file=$name" >>"$out"
else
  echo "file=$root/$name" >>"$out"
fi
