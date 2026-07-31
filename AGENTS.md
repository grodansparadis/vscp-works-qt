# AGENTS.md

This repository contains VSCP Works, a Qt/C++ desktop application built with CMake and Qt 6.8.1. Use this file as the default guide for coding agents working in this project.

## Project shape
- The main application code lives in [src](src), with Qt UI dialogs typically split across matching `.ui`, `.cpp`, and `.h` files.
- The vendored VSCP protocol/client implementation lives under [third_party/vscp/src](third_party/vscp/src); prefer small, compatibility-focused changes there.
- Build artifacts and generated files belong in [build](build) and should not be edited directly.

## Build and validation
- Configure and build from the repository root with:
  - `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug`
  - `cmake --build build --parallel`
- The workspace also provides a CMake build task labeled “CMake: build”; prefer that when working in VS Code.
- This project does not appear to have a top-level automated test suite wired into the root CMake setup, so rebuilding and checking for regressions is the main validation step.

## Conventions to follow
- Keep changes aligned with the existing Qt/C++ style used in [src](src) and [third_party/vscp/src](third_party/vscp/src).
- Do not modify files under [third_party](third_party) unless the task explicitly requires changes there.
- When changing a dialog or widget, update the corresponding `.ui` file and related source files together.
- Avoid introducing new dependencies unless they are already used by the project or clearly required.
- Prefer existing documentation in [README.md](README.md), [docs/setting_up_the_system.md](docs/setting_up_the_system.md), and [install/README.md](install/README.md) over duplicating guidance.

## Useful references
- [README.md](README.md) for the high-level product overview and build context.
- [CMakeLists.txt](CMakeLists.txt) for the build system and required toolchain dependencies.
- [docs](docs) for user-facing documentation and setup notes.
- The VSCP specification is available at https://grodansparadis.github.io/vscp-doc-spec/#/.
