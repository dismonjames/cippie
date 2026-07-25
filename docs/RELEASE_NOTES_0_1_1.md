# Cippie v0.1.1 Release Notes

Cippie v0.1.1 adds official installation and development workflow scripts, mark-based build graph execution, and adopts Cippie itself as the primary build system.

## What's New

- **Official Installer**: POSIX-compatible `scripts/install.sh` with SHA-256 verification, atomic binary replacement, and PATH management.
- **Uninstaller**: `scripts/uninstall.sh` for clean removal.
- **Development Scripts**: `scripts/build.sh`, `scripts/test.sh`, `scripts/check.sh`, and `scripts/self-host.sh` for streamlined development.
- **Source Sync Test**: `scripts/check-source-sync.py` ensures `.cpp` files in `CMakeLists.txt` match the `Cippiefile`.
- **Self-Hosting**: Cippie now builds itself using its own `Cippiefile` as the primary build system, with CMake as a bootstrap fallback.
- **Automated Releases**: GitHub Actions workflow now creates GitHub releases with assets and notes automatically on tag push.

## Supported Platforms & Compilers
- Linux (x86_64)
- GCC 13+ / Clang 17+ (C++23 support required)
