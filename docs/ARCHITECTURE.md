# Architecture

Cippie is designed with modularity, ownership clarity, and platform isolation.

## Subsystems

- `app`: High-level command dispatch and application lifecycle (`Application`).
- `cli`: Command-line parsing (`CommandLineParser`).
- `config`: Complete Cippiefile frontend DSL pipeline:
  - `Lexer`: Tokenizes Cippiefile DSL text with `SourceLocation` tracking.
  - `Parser`: Recursive descent AST parser with syntax error recovery.
  - `Ast`: Immutable AST representation (`AstValue`, `AstProjectDeclaration`, `AstTargetDeclaration`).
  - `Validator`: Strongly-typed validation and cycle detection.
  - `ConfigLoader`: Integrated file loader returning `Result<Project>`.
- `project`: Strongly typed models (`Project`, `Target`, `BuildConfiguration`, `Dependency`).
- `build`: Build graph creation, dependency scanning, compilation, and linking execution.
- `process`: Process execution abstraction (`ProcessRunner` with POSIX `fork`/`execvp`/`waitpid`).
- `toolchain`: Compiler, archiver, and linker detection (`ToolchainDetector`).
- `package`: Dependency resolution and package management.
- `cache`: Incremental build cache tracking.
- `diagnostics`: Diagnostics generation and rich terminal formatting (`DiagnosticPrinter`, `Logger`).
- `platform`: Operating system isolation (`linux/`, `windows/`, `macos/`).
- `util`: File IO and string utilities.
