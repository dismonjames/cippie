# Architecture

Cippie is designed with modularity, ownership clarity, and platform isolation.

## Subsystems

- `app`: High-level command dispatch and application lifecycle (`Application`).
- `cli`: Command-line parsing (`CommandLineParser`).
- `config`: Complete Cippiefile frontend DSL pipeline (`Lexer`, `Parser`, `Ast`, `Validator`, `ConfigLoader`).
- `project`: Strongly typed project models (`Project`, `Target`, `BuildConfiguration`, `Dependency`, `TargetSelector`, `ProjectGenerator`).
- `build`: Build graph & plan generation, deterministic source scanner, compile, archive & link command structures:
  - `SourceScanner`: Pattern-based glob expansion (`*.cpp`, `*.cc`, `*.cxx`, `**`), directory filtering, path normalization & sorting.
  - `BuildGraph`: Directed acyclic target graph with cycle detection and topological ordering.
  - `BuildPlanner`: Multi-target build planning, transitive public include propagation, `-fPIC` for shared libs, RPATH flags, and collision-free artifact paths.
  - `BuildEngine`: Multi-target build execution loop producing Ninja-formatted progress logs (`CXX`, `AR`, `LINK`).
- `process`: POSIX process execution abstraction (`ProcessRunner` with `fork`/`execvp`/`waitpid` isolation).
- `toolchain`: Compiler, archiver, linker, and target triple detection (`ToolchainDetector`, `TargetTriple`).
- `package`: Dependency resolution and package management.
- `cache`: Incremental build cache tracking.
- `diagnostics`: Diagnostics generation and rich terminal formatting (`DiagnosticPrinter`, `Logger`).
- `platform`: Operating system isolation (`linux/`, `windows/`, `macos/`).
- `util`: Shared filesystem, deletion safety (`CleanRunner`), and string utilities.

## Self-Hosting & Bootstrap Architecture

Cippie adopts a 2-generation self-hosting model:

1. **Bootstrap Phase**: CMake builds an initial temporary Cippie compiler binary (`build/cippie`).
2. **Generation 1 (Gen1)**: The bootstrap Cippie binary compiles Cippie from its own root `Cippiefile`.
3. **Generation 2 (Gen2)**: The Gen1 Cippie binary compiles Cippie again from `Cippiefile`.

CMake is maintained strictly as a temporary bootstrap fallback for initial source checkouts or broken host environments. Primary Cippie development uses `cippie build` and `cippie test`.
