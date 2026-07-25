# Cippie — Modern C++23 Build System and Package Manager

[![License: GPL-3.0-or-later](https://img.shields.io/badge/License-GPL_v3-blue.svg)](LICENSE)
[![Status: v0.1.0](https://img.shields.io/badge/Status-v0.1.0-green.svg)](docs/RELEASE_NOTES_0_1_0.md)

**Cippie** is a fast, deterministic C++23 build system, project manager, and package manager designed for modern C++ development without CMake wrapper magic.

## 1. Status

Current release: **v0.1.0 (Stable Technical Release)**.

Cippie successfully builds multi-target C++ projects, manages local and remote dependencies with lock files, cross-compiles across architectures, and **self-hosts** by building itself from its own `Cippiefile`.

## 2. Supported Systems & Requirements

- **Operating System**: Linux (x86_64, aarch64)
- **Compiler**: GCC 13+ or Clang 17+ with C++23 support (`-std=c++23`)
- **Build Tools**: CMake 3.25+ and Ninja (required for bootstrap compilation only)

## 3. Building from Source

```bash
# Clone the repository
git clone https://github.com/dismonjames/cippie.git
cd cippie

# Run the bootstrap script
./scripts/bootstrap.sh
```

## 4. Installation

```bash
# Install to /usr/local
sudo cmake --install build --prefix /usr/local

# Or install to user-local directory (~/.local)
cmake --install build --prefix "$HOME/.local"
```

## 5. Quick Start: Creating a Project

```bash
cippie new my_app
cd my_app

cippie build
cippie run
cippie test
```

## 6. Cippiefile Basic Example

`Cippiefile`:
```cpp
project("my_app") {
    cpp = 23;
    defaultTarget = "my_app";

    executable("my_app") {
        entry = "src/main.cpp";
        sources = ["src/**/*.cpp"];
        includes = ["include"];
    }
}
```

## 7. Multiple Targets & Dependencies

```cpp
project("multi_target_demo") {
    cpp = 23;

    static_library("core") {
        sources = ["core/src/*.cpp"];
        public_includes = ["core/include"];
    }

    executable("app") {
        entry = "apps/main.cpp";
        links = ["core"];
    }

    test("core_test") {
        entry = "tests/test_core.cpp";
        links = ["core"];
    }
}
```

## 8. Package Management

```bash
# Add a package dependency
cippie add math_lib@1.2.0

# Restore dependencies
cippie restore --locked
```

Top-level `dependencies` in `Cippiefile`:
```cpp
dependencies = [
    package("fmt", "10.1.0"),
    path_package("local_utils", "../local_utils"),
    git_package("my_lib", "https://github.com/user/my_lib.git", tag = "v1.0.0")
];
```

## 9. Incremental & Parallel Builds

Cippie automatically tracks header changes via compiler-generated `.d` files and SHA-256 content hashes.

```bash
# Build using 8 parallel worker threads
cippie build -j8

# Show toolchain diagnostics
cippie doctor
```

## 10. Cross-Compilation Example

```bash
# Cross-compile for ARM64 Linux
cippie build --target aarch64-linux-gnu
```

## 11. Self-Hosting

Cippie builds itself from its root `Cippiefile`:

```bash
# Build Cippie using the bootstrap Cippie binary
./build/cippie build cippie

# Verify 1st generation binary
./.cippie/build/x86_64-linux-gnu/debug/cippie/bin/cippie doctor

# Build 2nd generation using 1st generation binary
./.cippie/build/x86_64-linux-gnu/debug/cippie/bin/cippie build cippie
```

## 12. License

Distributed under the terms of the **GNU General Public License v3.0 or later** ([GPL-3.0-or-later](LICENSE)).
