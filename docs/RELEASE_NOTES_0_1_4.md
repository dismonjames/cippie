# Cippie v0.1.4 Release Notes

Cippie v0.1.4 adds full MSVC toolchain support for Windows, enabling the self-host build pipeline to work end-to-end on Windows with Microsoft Visual C++.

## What's New

- **MSVC toolchain detection**: Cippie now auto-detects `cl.exe`, `lib.exe`, and `link.exe` on Windows.
- **MSVC flag translation**: GCC-style warning flags (`-Wall`, `-Wextra`, `-Wpedantic`, `-Wconversion`, `-Wshadow`) translate to `/W4`; `-std=c++23` → `/std:c++latest`; `-g` → `/Zi`; `-fPIC` and `-Wl` flags are skipped.
- **MSVC artifact naming**: Uses `.obj` for objects, `.lib` for static libraries, `.dll` for shared libraries, `.exe` for executables.
- **MSVC archive support**: `lib.exe` invoked with `/NOLOGO /OUT:` format instead of `ar rcs`.
- **MSVC dependency generation**: Uses `/sourceDependencies` instead of `-MMD`/`-MP`/`-MF`/`-MT`.

## Supported Platforms & Compilers
- Linux (x86_64, aarch64) — GCC 13+ / Clang 17+
- macOS (x86_64, aarch64) — Apple Clang 15+
- Windows (x86_64) — MSVC 2022
