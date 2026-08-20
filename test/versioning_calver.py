#!/usr/bin/env python3
import re
import sys
from pathlib import Path

root = Path(__file__).resolve().parents[1]
version_h = root / "src" / "version.h"
buildnumber_h = root / "src" / "buildnumber.h"

if not version_h.exists():
    print(f"Missing version header: {version_h}", file=sys.stderr)
    sys.exit(1)

text = version_h.read_text(encoding="utf-8")
assert re.search(r"#define\s+VSCPWORKS_MAJOR_VERSION\s+([0-9]+)", text), "MAJOR macro missing"
assert re.search(r"#define\s+VSCPWORKS_MINOR_VERSION\s+([0-9]+)", text), "MINOR macro missing"
assert re.search(r"#define\s+VSCPWORKS_PATCH_VERSION\s+([0-9]+)", text), "PATCH macro missing"
assert "VSCPWORKS_RELEASE_VERSION" not in text, "Legacy RELEASE macro should not be used"

build_text = buildnumber_h.read_text(encoding="utf-8")
match = re.search(r'#define\s+VSCPWORKS_DISPLAY_VERSION\s+"([0-9]+)\.([0-9]+)\.([0-9]+)"', build_text)
assert match, "DISPLAY_VERSION should follow MAJOR.MINOR.PATCH"
major, minor, patch = map(int, match.groups())
assert 10 <= major <= 99, "MAJOR should be a two-digit calendar year"
assert minor >= 0, "MINOR must be non-negative"
assert patch >= 0, "PATCH must be non-negative"

print("versioning_calver: OK")
