# Cippie v0.1.3 Release Notes

Cippie v0.1.3 fixes Windows build compatibility and streamlines the release pipeline.

## What's Fixed

- **Windows support**: Added native Windows process runner using `CreateProcess`/`WaitForSingleObject` instead of POSIX `fork`/`exec`. Fixed file locking (`_locking` instead of `flock`).
- **Self-hosting release pipeline**: Release workflow now downloads the previous Cippie release and uses it to build the new version — no CMake required in CI. Each release is built by the prior release.
- **`package-release.sh`**: Now supports `--cippie-bin`, `--cmake-bootstrap`, and auto-download of the latest release when no Cippie binary is available.

## Supported Platforms & Compilers
- Linux (x86_64, aarch64) — GCC 13+ / Clang 17+
- macOS (x86_64, aarch64) — Apple Clang 15+
- Windows (x86_64) — MSVC 2022 / MinGW
