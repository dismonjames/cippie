# Cippie Development Roadmap

## Released: v0.1.0 (Initial Stable Release)
- Declarative C-style `Cippiefile` parser, AST, and validator.
- Multi-target dependency graphs with cycle detection.
- Parallel worker thread scheduler (`-j` / `--jobs`).
- Content-aware build caching via compiler `.d` files & SHA-256 hashes.
- Local path packages, Git dependencies, semantic versioning, lock files, and offline mode.
- Target triple parsing, custom toolchains, and `cippie doctor`.
- 2-generation self-hosting capability.

## Post-v0.1.0 Improvements
- Official end-user Linux binary installer script (`scripts/install.sh`) with SHA-256 verification and atomic replacement.
- Uninstaller script (`scripts/uninstall.sh`).
- Enhanced 2-generation self-host verification script (`scripts/self-host.sh`).
- Primary development workflows via Cippie (`cippie build`, `cippie test`), maintaining CMake strictly as a bootstrap fallback.
- Production source file synchronization test (`scripts/check-source-sync.py`).

## Future Milestones
- Windows MSVC native toolchain detection.
- macOS Darwin target support.
- Planned `cippie update` command for binary self-updates.
