# Changelog

All notable changes to Cippie will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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
