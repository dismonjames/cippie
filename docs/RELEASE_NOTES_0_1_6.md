# Cippie v0.1.6 Release Notes

Cippie v0.1.6 is a patch release fixing MSVC compiler option translation and updating project versioning across build scripts, installers, and test suites.

## What's New & Fixed

- **Fixed MSVC `-std=c++` Flag Translation**: Resolved a substring calculation bug (`substr(8)` vs `substr(9)`) in `BuildPlanner.cpp` that prevented `-std=c++23` from translating to `/std:c++latest` when building with MSVC on Windows.
- **Enhanced Compiler Flag Mapping**: Expanded `translateOptions` to handle `/std:c++latest`, `/std:c++20`, `/std:c++17`, and `/std:c++14` for all supported C++ standard options.
- **Unit Test Coverage**: Added unit tests in `BuildPlannerTests` to ensure MSVC flag translations are verified automatically.
- **Version Alignment**: Updated version string to `0.1.6` across `CMakeLists.txt`, `Version.hpp`, installer scripts (`install.sh`, `install.ps1`), `package-release.sh`, and `VersionConsistencyTests.cpp`.

## Supported Platforms & Compilers
- Linux (x86_64, aarch64) — GCC 13+ / Clang 17+
- macOS (x86_64, aarch64) — Apple Clang 15+
- Windows (x86_64) — MSVC 2022
