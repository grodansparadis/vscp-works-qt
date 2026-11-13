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
# Written for bash 3.2 too (the bash on macOS runners): no associative
# arrays, globstar, mapfile or ${var,,}.
#

set -euo pipefail
shopt -s nullglob dotglob

name="${1:-SHA256SUMS}"
name=$(printf '%s' "$name" | tr -c 'A-Za-z0-9._-' '-')
out="${GITHUB_OUTPUT:-/dev/stdout}"
cd "${GITHUB_WORKSPACE:-.}"
ws="${GITHUB_WORKSPACE:-$(pwd)}"
ws="${ws//\\//}"
ws="${ws%/}"

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

lc() {
  printf '%s' "$1" | tr '[:upper:]' '[:lower:]'
}

trim() {
  local s="$1"
  s="${s#"${s%%[![:space:]]*}"}"
  s="${s%"${s##*[![:space:]]}"}"
  printf '%s' "$s"
}

# Make a pattern relative to the workspace when it points inside it. The
# comparison ignores case so Windows drive letters (D:/ vs d:/) match.
relative() {
  local p="${1//\\//}" lp lws
  p="${p#./}"
  lp=$(lc "$p")
  lws=$(lc "$ws")
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
  local IFS='/'
  set -f
  # shellcheck disable=SC2086
  set -- $p
  set +f
  for seg in "$@"; do
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

# Expand a pattern to files, one per line (directories expand to all files
# below them). '**' is handled with find, since bash 3.2 has no globstar.
expand() {
  local p="$1" m root fp
  case "$p" in
    *'**'*)
      root=$(search_root "$p")
      [ -d "$root" ] || return 0
      # find -path lets '*' match '/', so '**/' and '/**' become '*'
      fp="${p//\*\*\//*}"
      fp="${fp//\/\*\*/\/*}"
      fp="${fp//\*\*/*}"
      find "$root" -type f -path "$fp"
      return 0
      ;;
  esac
  local IFS=$'\n'
  local matches
  # shellcheck disable=SC2206
  matches=($p)
  if [ ${#matches[@]} -eq 0 ] && [ -e "$p" ]; then
    matches=("$p")
  fi
  for m in "${matches[@]+"${matches[@]}"}"; do
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

: >"$work/excluded"
for p in "${excludes[@]+"${excludes[@]}"}"; do
  expand "$p" | sed 's#^\./##' >>"$work/excluded"
done

: >"$work/files"
for p in "${includes[@]+"${includes[@]}"}"; do
  expand "$p" | sed 's#^\./##' >>"$work/files"
done

files=()
while IFS= read -r f; do
  [ -z "$f" ] && continue
  case "$(basename "$f")" in SHA256SUMS*) continue ;; esac
  if grep -qxF -- "$f" "$work/excluded"; then
    continue
  fi
  files+=("$f")
done < <(awk '!seen[$0]++' "$work/files")

if [ ${#files[@]} -eq 0 ]; then
  echo "write-sha256sums: no files matched, nothing to do"
  echo "file=" >>"$out"
  exit 0
fi

# Artifact root as upload-artifact computes it: the search paths of the
# patterns, without paths below another one; a single file uses its directory,
# several search paths use their common ancestor.
for p in "${includes[@]}"; do
  search_root "$p"
  echo
done | sort -u >"$work/roots"
kept=()
while IFS= read -r r; do
  [ -z "$r" ] && continue
  inside=0
  while IFS= read -r o; do
    if [ -z "$o" ] || [ "$o" = "$r" ]; then continue; fi
    if [ "$o" = "." ] || [ "${r#"$o"/}" != "$r" ]; then inside=1; break; fi
  done <"$work/roots"
  [ "$inside" = 0 ] && kept+=("$r")
done <"$work/roots"

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

tmp="$work/sums"
if command -v sha256sum >/dev/null; then
  (cd "$root" && sha256sum -b "${rel[@]}") >"$tmp"
else
  (cd "$root" && shasum -a 256 -b "${rel[@]}") >"$tmp"
fi
# Readable by everyone: upload-artifact must be able to read it even when the
# file ends up owned by root (build output written from a Docker container)
chmod 0644 "$tmp"
if [ -w "$root" ]; then
  cp "$tmp" "$root/$name"
else
  sudo cp "$tmp" "$root/$name"
  sudo chmod 0644 "$root/$name"
fi
cat "$root/$name"

if [ "$root" = "." ]; then
  echo "file=$name" >>"$out"
else
  echo "file=$root/$name" >>"$out"
fi
