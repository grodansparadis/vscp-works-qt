# Copilot instructions for VSCP Works

## Repository overview
- This repository builds a Qt/C++ desktop application named VSCP Works with CMake and Qt 6.8.1.
- The main UI lives in [src](src); the vendored protocol/client implementation lives in [third_party/vscp/src](third_party/vscp/src).
- Prefer small, compatibility-focused changes in the vendored VSCP code and keep application changes aligned with the existing Qt style.

## Build and validation
- Build from the repository root with `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug` followed by `cmake --build build --parallel`.
- If you are working in VS Code, prefer the existing CMake build task labeled “CMake: build”.
- Rebuild after meaningful changes and check for regressions; this project does not appear to have a root-level automated test suite wired into CMake.

## Editing guidance
- Do not edit files under [third_party](third_party) unless the task explicitly requires changes there.
- When changing a Qt dialog or widget, update the matching `.ui`, `.cpp`, and `.h` files together.
- Avoid introducing new dependencies unless they are already used by the project or clearly required.
- Do not edit generated build artifacts under [build](build).

## Useful references
- [README.md](README.md)
- [CMakeLists.txt](CMakeLists.txt)
- [docs/setting_up_the_system.md](docs/setting_up_the_system.md)
- [install/README.md](install/README.md)
- The VSCP specification is available at https://grodansparadis.github.io/vscp-doc-spec/#/.
