# Cippie v0.1.2 Release Notes

Cippie v0.1.2 adds a self-update command, cross-platform support, and automated multi-platform releases.

## What's New

- **`cippie update`** — Automatically checks for the latest release on GitHub, downloads, verifies, and replaces the current binary. Periodic check every 18 days (use `--force` to override).
- **Windows support** — `scripts/install.ps1` PowerShell installer for Windows.
- **macOS support** — `install.sh` now works on macOS (darwin) in addition to Linux.
- **Self-hosted builds** — Release packaging uses Cippie's own build system (bootstrap CMake → self-host), ensuring the released binary is built by Cippie itself.
- **Multi-platform releases** — GitHub Actions now builds and uploads native binaries for Linux (x86_64), macOS (x86_64, aarch64), and Windows (x86_64) on every release.

## Supported Platforms & Compilers
- Linux (x86_64, aarch64)
- macOS (x86_64, aarch64)
- Windows (x86_64)
- GCC 13+ / Clang 17+ / Apple Clang 15+ / MSVC 2022 (C++23 support required)
