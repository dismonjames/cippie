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
