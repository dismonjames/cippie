# Changelog

## Unreleased

### Added

- Incremental builds and parallel graph execution (Milestones 4 & 5):
  - Streaming FNV-1a 64-bit content hasher (`FileHasher`) with in-memory memoization.
  - Compiler dependency file parser (`DepFileParser`) supporting Makefile continuations (`\`), escaped spaces (`\ `), duplicate removal, and safe malformed file recovery.
  - Content-aware deterministic cache keys (`CacheKeyBuilder`) covering source hash, header dependency hashes, compiler identity, options, C++ standard, PIC mode, target triple, and configuration.
  - Persistent manifest store (`ManifestStore`) with atomic file writes (`.tmp` + rename), schema validation, and stale entry pruning.
  - Parallel task execution engine (`TaskScheduler` & `ThreadPool`) executing graph nodes concurrently up to worker pool capacity (`-j N`, `--jobs N`).
  - Thread-safe non-blocking process output capture (`LinuxProcessRunner`) preventing pipe buffer deadlocks and emitting contiguous atomic failure blocks.
  - Project-local build lock (`BuildLock`) using `flock(fd, LOCK_EX | LOCK_NB)` on `.cippie/locks/build.lock`.
  - Cache clean options (`cippie clean --cache` and `cippie clean --all`).
  - 37 total unit and integration tests passing under AddressSanitizer and UndefinedBehaviorSanitizer.
- Complete target system, project generation, test runner, and clean workflow (Milestones 3 & 6).
- Real single-target build and run pipeline (Milestone 2).
- Complete Cippiefile frontend (Milestone 1).
- Initial project architecture and bootstrap CMake build.
