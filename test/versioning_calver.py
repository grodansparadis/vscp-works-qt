#!/usr/bin/env python3
import re
import sys
from pathlib import Path

root = Path(__file__).resolve().parents[1]
version_h = root / "src" / "version.h"

if not version_h.exists():
    print(f"Missing version header: {version_h}", file=sys.stderr)
    sys.exit(1)

text = version_h.read_text(encoding="utf-8")
assert re.search(r"#define\s+VSCPWORKS_MAJOR_VERSION\s+([0-9]+)", text), "MAJOR macro missing"
assert re.search(r"#define\s+VSCPWORKS_MINOR_VERSION\s+([0-9]+)", text), "MINOR macro missing"
assert re.search(r"#define\s+VSCPWORKS_PATCH_VERSION\s+([0-9]+)", text), "PATCH macro missing"
assert "VSCPWORKS_RELEASE_VERSION" not in text, "Legacy RELEASE macro should not be used"

match = re.search(r'#define\s+VSCPWORKS_DISPLAY_VERSION\s+"([0-9]+)-([0-9]+)-([0-9]+)"', text)
assert match, "DISPLAY_VERSION should follow YEAR-MONTH-PATCH"
major, minor, patch = map(int, match.groups())
assert 10 <= major <= 99, "MAJOR should be a two-digit calendar year"
assert 1 <= minor <= 12, "MINOR should be the calendar month"
assert patch >= 0, "PATCH must be non-negative"
macro_versions = tuple(
    int(re.search(rf"#define\s+VSCPWORKS_{component}_VERSION\s+([0-9]+)", text)[1])
    for component in ("MAJOR", "MINOR", "PATCH")
)
assert (major, minor, patch) == macro_versions, (
    "DISPLAY_VERSION should match the numeric version macros"
)

print("versioning_calver: OK")
