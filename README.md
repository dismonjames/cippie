# Cippie — Modern C++23 Build System and Package Manager

[![License: GPL-3.0-or-later](https://img.shields.io/badge/License-GPL_v3-blue.svg)](LICENSE)
[![Status: v0.1.0](https://img.shields.io/badge/Status-v0.1.0-green.svg)](docs/RELEASE_NOTES_0_1_0.md)

**Cippie** is a fast, deterministic C++23 build system, project manager, and package manager designed for modern C++ development without CMake wrapper magic.

## 1. Quick Start

Install the official prebuilt Cippie Linux binary:

```bash
curl -fsSL https://raw.githubusercontent.com/dismonjames/cippie/main/scripts/install.sh | sh
```

Or review the installer before execution:

```bash
curl -fsSL https://raw.githubusercontent.com/dismonjames/cippie/main/scripts/install.sh -o install-cippie.sh
less install-cippie.sh
sh install-cippie.sh
```

Create and run a new project:

```bash
cippie new hello
cd hello
cippie build
cippie run
cippie test
```

## 2. Installation Options

### Option A — Install Official Prebuilt Binary

```bash
# Default installation to $HOME/.local/bin/cippie
./scripts/install.sh

# Install specific version
./scripts/install.sh --version 0.1.0

# Install to custom directory prefix
./scripts/install.sh --prefix "$HOME/.local"

# Force overwrite existing binary
./scripts/install.sh --force
```

### Option B — Install from Source (Bootstrap)

```bash
git clone https://github.com/dismonjames/cippie.git
cd cippie

# Build Cippie using Cippie self-host pipeline and install
./scripts/bootstrap.sh --prefix "$HOME/.local"
```

## 3. Uninstalling Cippie

```bash
./scripts/uninstall.sh --prefix "$HOME/.local"
```

*Note: The uninstaller preserves user configuration and package caches (`~/.cache/cippie`, `~/.config/cippie`).*

## 4. Primary Development Workflow

Cippie is self-hosting and uses Cippie as its primary development build system:

```bash
# Build Cippie using Cippie
cippie build

# Run Cippie tests
cippie test

# Run quality checks (source sync, unit tests, self-host 2-gen verification)
./scripts/check.sh
```

*CMake is maintained strictly as a temporary bootstrap and recovery fallback.*

## 5. PATH Troubleshooting

If `cippie` is not recognized after installation, ensure `$HOME/.local/bin` is in your `PATH`:

```bash
export PATH="$HOME/.local/bin:$PATH"
```

Add the line above to your `~/.bashrc` or `~/.zshrc` to make it permanent.

## 6. Self-Hosting Verification

Verify 2-generation self-host compilation:

```bash
./scripts/self-host.sh --release
```

## 7. License

Distributed under the terms of the **GNU General Public License v3.0 or later** ([GPL-3.0-or-later](LICENSE)).
