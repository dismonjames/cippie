# Cippie v0.1.0 Release Notes

Cippie v0.1.0 is the first stable technical release of **Cippie**, a modern C++23 build system, project manager, and package manager.

## Key Features

- **Cippiefile Frontend**: Declarative C-style configuration parser, validator, and diagnostic formatter with precise caret reporting.
- **Multi-Target Graph Execution**: Support for executable, static library, shared library, and test targets with cycle detection and topological ordering.
- **Content-Aware Incremental Builds**: Header dependency tracking via compiler-generated `.d` files and SHA-256 content hashing.
- **Parallel Graph Execution**: Multi-threaded build scheduler (`-j` / `--jobs`).
- **Package Management**: Local path packages, Git dependencies, semantic versioning, lock files (`Cippie.lock`), and offline restoration.
- **Toolchains & Cross-Compilation**: Target triple normalization (`x86_64-linux-gnu`, `aarch64-linux-gnu`), toolchain registry (`~/.config/cippie/toolchains/`), and cross-build artifact isolation.
- **Environment Diagnostics**: `cippie doctor` command for inspecting system compiler capabilities, caches, and toolchain configurations.
- **Self-Hosting**: Full 2-generation self-hosting capability where Cippie builds itself from its own `Cippiefile`.

## Supported Platforms & Compilers
- Linux (x86_64, aarch64)
- GCC 13+ / Clang 17+ (C++23 support required)
