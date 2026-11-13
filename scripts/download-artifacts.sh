#!/usr/bin/env bash
#
# download-artifacts.sh
#
# Download the build artifacts of the newest successful run of every GitHub
# Actions workflow in this repository, and check them against the checksum
# files they contain (SHA256SUMS, or per-file *.sha256).
#
# Requires the GitHub CLI (https://cli.github.com), signed in with
# "gh auth login". GitHub only serves workflow artifacts to signed-in users,
# also for public repositories. Artifacts are kept for 90 days.
#
# Usage:
#   scripts/download-artifacts.sh [-b branch] [-o dir] [-R owner/repo] [-w workflow] [filter...]
#
#   -b branch    Branch to take builds from (default: the repository's default branch)
#   -o dir       Output directory (default: artifacts). Each artifact goes in its own folder.
#   -R repo      GitHub repository (default: from the 'origin' remote)
#   -w workflow  Only this workflow file, for example build-linux.yml (repeatable)
#   filter       Only artifacts whose name contains one of these words (any case)
#
# Examples:
#   scripts/download-artifacts.sh
#   scripts/download-artifacts.sh linux windows
#   scripts/download-artifacts.sh -b development -o /tmp/builds
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

REPO=""
BRANCH=""
OUTDIR="artifacts"
WORKFLOWS=()

usage() {
  sed -n '3,26p' "$0" | sed 's/^# \{0,1\}//'
  exit "${1:-0}"
}

while getopts "b:o:R:w:h" opt; do
  case "$opt" in
    b) BRANCH="$OPTARG" ;;
    o) OUTDIR="$OPTARG" ;;
    R) REPO="$OPTARG" ;;
    w) WORKFLOWS+=("$OPTARG") ;;
    h) usage 0 ;;
    *) usage 1 ;;
  esac
done
shift $((OPTIND - 1))
FILTERS=("$@")

if ! command -v gh >/dev/null; then
  echo "The GitHub CLI (gh) is required: https://cli.github.com" >&2
  exit 1
fi
if ! gh auth status >/dev/null 2>&1; then
  echo "Sign in to GitHub first with: gh auth login" >&2
  exit 1
fi

if [ -z "$REPO" ]; then
  url=$(git -C "$(dirname "$0")" remote get-url origin 2>/dev/null ||
        git remote get-url origin 2>/dev/null || true)
  REPO=$(printf '%s' "$url" | sed -E 's#^(https?://|git@|ssh://git@)github\.com[:/]##; s#\.git$##; s#/$##')
  if [ -z "$REPO" ] || [ "$REPO" = "$url" ]; then
    echo "Can't tell the GitHub repository from the origin remote; use -R owner/repo" >&2
    exit 1
  fi
fi
if [ -z "$BRANCH" ]; then
  BRANCH=$(gh repo view "$REPO" --json defaultBranchRef --jq .defaultBranchRef.name)
fi

if [ ${#WORKFLOWS[@]} -eq 0 ]; then
  mapfile -t WORKFLOWS < <(gh workflow list -R "$REPO" --json path,state \
    --jq '.[] | select(.state == "active") | .path | split("/") | last')
fi

wanted() {
  local name="${1,,}" f
  [ ${#FILTERS[@]} -eq 0 ] && return 0
  for f in "${FILTERS[@]}"; do
    case "$name" in *"${f,,}"*) return 0 ;; esac
  done
  return 1
}

sha256_of() {
  if command -v sha256sum >/dev/null; then
    sha256sum -b "$1" | cut -d' ' -f1
  else
    shasum -a 256 -b "$1" | cut -d' ' -f1
  fi
}

# File names listed in a checksum file ("hash  name" or "hash *name")
listed_files() {
  sed -nE 's/^[0-9a-fA-F]{64} [ *](.*)$/\1/p' "$1" | tr -d '\r'
}

# Check every entry of one checksum file against files below 'base'. An entry
# whose file isn't next to the checksum file is looked up by name in the other
# downloaded artifacts (some workflows upload checksums as a separate
# artifact). Returns 1 on a mismatch.
check_sums() {
  local sums="$1" dir hash name file actual rc=0 n=0
  dir=$(dirname "$sums")
  while IFS= read -r line; do
    line="${line%$'\r'}"
    [[ "$line" =~ ^([0-9a-fA-F]{64})\ [\ *](.+)$ ]] || continue
    hash="${BASH_REMATCH[1],,}"
    name="${BASH_REMATCH[2]}"
    file="$dir/$name"
    if [ ! -f "$file" ]; then
      file=$(find "$OUTDIR" -type f -name "$(basename "$name")" -print -quit)
    fi
    if [ -z "$file" ]; then
      echo "  $name: not downloaded, skipped"
      continue
    fi
    actual=$(sha256_of "$file")
    n=$((n + 1))
    if [ "$actual" = "$hash" ]; then
      echo "  $name: OK"
    else
      echo "  $name: CHECKSUM MISMATCH" >&2
      rc=1
    fi
  done <"$sums"
  [ "$n" -gt 0 ] && verified=1
  return $rc
}

mkdir -p "$OUTDIR"
found=0
failed=0
verified=0
declare -A used=()
downloaded=()

for wf in "${WORKFLOWS[@]}"; do
  run=$(gh run list -R "$REPO" --workflow "$wf" --branch "$BRANCH" --status success \
          --limit 1 --json databaseId,headSha,createdAt \
          --jq '.[0] | select(. != null) | "\(.databaseId) \(.headSha[0:7]) \(.createdAt)"' 2>/dev/null || true)
  [ -z "$run" ] && continue
  read -r run_id sha created <<<"$run"

  # github-pages is a site deployment, not a build
  mapfile -t names < <(gh api "repos/$REPO/actions/runs/$run_id/artifacts" --paginate \
    --jq '.artifacts[] | select(.expired | not) | select(.name != "github-pages") | .name')
  for name in "${names[@]+"${names[@]}"}"; do
    wanted "$name" || continue
    found=1
    dest="$OUTDIR/$name"
    if [ -n "${used[$dest]+x}" ]; then
      dest="$OUTDIR/$name-${wf%.*}"
    fi
    used["$dest"]=1
    rm -rf "$dest"
    mkdir -p "$dest"
    echo "$name: $wf run $run_id, commit $sha, built $created"
    if gh run download "$run_id" -R "$REPO" -n "$name" -D "$dest" >/dev/null; then
      downloaded+=("$dest")
    else
      echo "  download failed" >&2
      failed=1
    fi
  done
done

if [ "$found" = 0 ]; then
  echo "No artifacts found on $BRANCH${FILTERS[*]:+ matching: ${FILTERS[*]}}" >&2
  exit 1
fi

# Verify after everything is downloaded, so checksums in one artifact can
# cover files in another
echo
echo "Checksums:"
for dest in "${downloaded[@]+"${downloaded[@]}"}"; do
  mapfile -t sums < <(find "$dest" -type f \( -name 'SHA256SUMS*' -o -name '*.sha256' \) | sort)
  if [ ${#sums[@]} -eq 0 ]; then
    echo "$(basename "$dest"): no checksum file, not verified"
    continue
  fi
  for f in "${sums[@]}"; do
    echo "$(basename "$dest")/${f#"$dest"/}"
    check_sums "$f" || failed=1
  done
done
if [ "$verified" = 0 ]; then
  echo "No checksums to verify"
fi

exit "$failed"
