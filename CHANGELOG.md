# Changelog

All notable changes to Cippie will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.1.3] - 2026-07-25

### Fixed
- Windows build compatibility: added native Windows process runner (`CreateProcess`), fixed POSIX file locking for Windows.
- Release workflow now uses previous Cippie release to self-host build, removing CMake dependency from CI.

## [0.1.2] - 2026-07-25

### Added
- `cippie update` command — auto-update to the latest release with periodic check (18-day cooldown).
- Cross-platform builds: Windows (install.ps1) and macOS support.
- Self-hosted release packaging: `package-release.sh` now builds via Cippie's own build system.
- CI now builds and releases native binaries for Linux, macOS, and Windows.

## [0.1.1] - 2026-07-25

### Added
- Official end-user Linux binary installer script (`scripts/install.sh`) with SHA-256 verification and atomic replacement.
- Uninstaller script (`scripts/uninstall.sh`).
- Enhanced 2-generation self-hosting verification script (`scripts/self-host.sh`).
- Primary Cippie development workflow scripts (`scripts/build.sh`, `scripts/test.sh`, `scripts/check.sh`).
- Production source file synchronization test (`scripts/check-source-sync.py`).
- Cippie adopted as the primary build system for developing Cippie itself, keeping CMake strictly as a bootstrap fallback.
- Automated releases: GitHub Actions now builds and publishes releases with assets for all platforms automatically.

## [0.1.0] - 2026-07-25

### Added
- Complete C-style `Cippiefile` parser, AST, and validator with source caret diagnostics.
- Target support: `executable`, `static_library`, `shared_library`, `test`.
- Multi-target dependency graph planner with cycle detection and topological ordering.
- Parallel worker thread pool scheduler (`-j` / `--jobs`).
- Content-aware build cache with SHA-256 hashing and compiler-generated `.d` header tracking.
- Dependency and package manager supporting local path packages, Git dependencies, semantic versioning, lock files (`Cippie.lock`), and offline mode.
- Target triple parsing (`Arch`, `Os`, `Abi`) and toolchain registry (`~/.config/cippie/toolchains/`).
- `cippie doctor` diagnostic command.
- Self-hosting build system configuration (`Cippiefile`) enabling Cippie to build itself across two generations.
- Release packaging script (`scripts/package-release.sh`) and bootstrap installer (`scripts/bootstrap.sh`).
