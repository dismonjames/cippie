# Architecture

Cippie is designed with modularity, ownership clarity, and platform isolation.

## Subsystems

- `app`: High-level command dispatch and application lifecycle (`Application`).
- `cli`: Command-line parsing (`CommandLineParser`).
- `config`: Complete Cippiefile frontend DSL pipeline (`Lexer`, `Parser`, `Ast`, `Validator`, `ConfigLoader`).
- `project`: Strongly typed project models (`Project`, `Target`, `BuildConfiguration`, `Dependency`, `TargetSelector`).
- `build`: Build graph & plan generation, deterministic source scanner, compile & link command structures:
  - `SourceScanner`: Pattern-based glob expansion (`*.cpp`, `*.cc`, `*.cxx`, `**`), directory filtering, path normalization & sorting.
  - `BuildPlanner`: Collision-free object mapping and structured `CompileCommand` and `LinkCommand` build plans.
  - `BuildEngine`: Sequential build execution loop producing Ninja-formatted progress logs.
- `process`: POSIX process execution abstraction (`ProcessRunner` with `fork`/`execvp`/`waitpid` isolation).
- `toolchain`: Compiler, archiver, linker, and target triple detection (`ToolchainDetector`, `TargetTriple`).
- `package`: Dependency resolution and package management.
- `cache`: Incremental build cache tracking.
- `diagnostics`: Diagnostics generation and rich terminal formatting (`DiagnosticPrinter`, `Logger`).
- `platform`: Operating system isolation (`linux/`, `windows/`, `macos/`).
- `util`: Shared filesystem and string utilities.
