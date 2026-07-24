# Cippie Cache & Parallel Execution Model

This document specifies the incremental compilation cache, header dependency tracking, persistent manifest model, lock safety, and parallel task execution engine in Cippie.

## 1. Persistent Directory Architecture

Persistent build artifacts, manifests, caches, and process locks are stored under `.cippie/`:

```text
.cippie/
├── build/<triple>/<config>/<target>/
│   ├── bin/
│   ├── lib/
│   ├── obj/
│   └── dep/
├── manifests/<triple>/<config>/<target>/
│   └── manifest.txt
└── locks/
    └── build.lock
```

- **Build Output**: Isolated per target triple, build configuration, and target.
- **Manifests**: Stores persistent cache entries containing normalized source path, object path, depfile path, deterministic content-aware cache key, and header dependency path-hash pairs.
- **Locks**: Process-level lock file ensuring safe concurrent Cippie execution.

## 2. Header Dependency Tracking

During compilation, GCC and Clang generate dependency files (`.d`) via:
- `-MMD -MP -MF <depfile> -MT <objectfile>`

`DepFileParser`:
- Parses Makefile rule syntax.
- Handles line continuations (`\`), escaped spaces (`\ `), and escaped special characters (`\#`, `\:`, `\\`).
- Deduplicates and normalizes dependency paths.
- Missing or malformed `.d` files trigger safe recompilation without crashing.

## 3. Content-Aware Cache Keys

A compile step is up to date only when all inputs match. Cache invalidation considers:
- Source file content hash (FNV-1a 64-bit streamed hash).
- Header dependency content hashes.
- Compiler identity (path, version, family).
- Target triple and configuration (`debug` vs `release`).
- C++ standard version (`cpp = 23`).
- Include directories, private & public compile definitions, and options.
- Position-Independent Code (`-fPIC`) requirement.

Touching a source file without changing its content or changing unrelated sources does **not** invalidate the cache key.

## 4. Parallel Graph Scheduler

`TaskScheduler` & `ThreadPool`:
- DAG execution of `CompileNode`, `ArchiveNode`, and `LinkNode`.
- Worker pool capped safely, defaulting to `std::thread::hardware_concurrency()`.
- Job control flags: `-j N`, `-jN`, `--jobs N`, `--jobs=N`.
- Failure propagation: Failing a node marks transitive dependent nodes as `blocked` while allowing independent graph branches to complete.
- Process output capture: Concurrently reads stdout/stderr using non-blocking I/O to avoid pipe buffer deadlocks, outputting contiguous atomic error blocks on step completion.

## 5. Process-Local Lock

`BuildLock`:
- Uses non-blocking `flock(fd, LOCK_EX | LOCK_NB)` on `.cippie/locks/build.lock`.
- Prevents concurrent Cippie build processes from corrupting build manifests.
- Releases lock automatically via RAII handles.

## 6. Clean Modes

- `cippie clean`: Removes `.cippie/build`.
- `cippie clean --cache`: Removes `.cippie/cache` and `.cippie/manifests` (forces full rebuild).
- `cippie clean --all`: Removes entire `.cippie/` directory.
