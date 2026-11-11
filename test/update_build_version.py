#!/usr/bin/env python3
import os
import re
import shutil
import subprocess
import tempfile
from datetime import datetime, timezone
from pathlib import Path

root = Path(__file__).resolve().parents[1]


def check_numeric_versions(header):
    for component in ("MAJOR", "MINOR", "PATCH"):
        match = re.search(
            rf"#define\s+VSCPWORKS_{component}_VERSION\s+([0-9]+)", header
        )
        assert match, f"{component} macro missing"
        assert re.fullmatch(r"0|[1-9][0-9]*", match[1]), (
            f"{component} must be a decimal literal without leading zeros: {match[1]}"
        )


check_numeric_versions((root / "src" / "version.h").read_text(encoding="utf-8"))

with tempfile.TemporaryDirectory() as directory:
    source = Path(directory)
    (source / "src").mkdir()
    for name in ("version.h", "buildnumber.h"):
        shutil.copyfile(root / "src" / name, source / "src" / name)

    for month in range(1, 13):
        env = os.environ.copy()
        env["SOURCE_DATE_EPOCH"] = str(
            int(datetime(2026, month, 15, tzinfo=timezone.utc).timestamp())
        )
        env["TZ"] = "UTC"
        subprocess.run(
            [
                "cmake",
                f"-DSOURCE_DIR={source}",
                "-P",
                str(root / "cmake" / "update_build_version.cmake"),
            ],
            env=env,
            check=True,
        )
        header = (source / "src" / "version.h").read_text(encoding="utf-8")
        check_numeric_versions(header)
        match = re.search(r"#define\s+VSCPWORKS_MINOR_VERSION\s+([0-9]+)", header)
        assert match[1] == str(month), "Numeric month should be unpadded"
        display = (source / "src" / "buildnumber.h").read_text(encoding="utf-8")
        assert re.search(
            rf'#define\s+VSCPWORKS_DISPLAY_VERSION\s+"26-{month:02d}-[0-9]+"',
            display,
        ), "Display month should remain zero-padded"

print("update_build_version: OK")
