#!/usr/bin/env python3
import json
import os
import sys
from pathlib import Path


def load_compile_commands(path: Path):
    with path.open("r", encoding="utf-8") as handle:
        return json.load(handle)


def is_supported_source(entry: dict) -> bool:
    file_path = entry.get("file", "")
    if not file_path:
        return False
    suffix = os.path.splitext(file_path)[1].lower()
    return suffix in {".c", ".cc", ".cpp", ".cxx", ".c++"}


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: source_syntax_smoke.py <compile_commands.json>", file=sys.stderr)
        return 2

    compile_commands_path = Path(sys.argv[1]).resolve()
    if not compile_commands_path.exists():
        print(f"compile commands file not found: {compile_commands_path}", file=sys.stderr)
        return 2

    commands = load_compile_commands(compile_commands_path)
    sources = [entry for entry in commands if is_supported_source(entry)]

    if not sources:
        print("no C/C++ compile entries found", file=sys.stderr)
        return 2

    missing = []
    for entry in sources:
        file_path = Path(entry.get("file", ""))
        if not file_path.is_absolute():
            file_path = (compile_commands_path.parent / file_path).resolve()
        if not file_path.exists():
            missing.append(str(file_path))

        command = entry.get("command", "")
        if not command:
            missing.append(f"missing-command:{entry.get('file', '')}")

    if missing:
        print("compile database validation failed", file=sys.stderr)
        for item in missing[:10]:
            print(f" - {item}", file=sys.stderr)
        return 1

    print(f"validated {len(sources)} source entries from the compile database")
    return 0


if __name__ == "__main__":
    main()
