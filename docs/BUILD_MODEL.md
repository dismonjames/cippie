# Cippie Build Model

This document specifies the build pipeline, target selection logic, object path layout, and execution model implemented in Cippie.

## Target Selection Pipeline

When `cippie build [target]` or `cippie run [target]` is invoked:

1. **Explicit Target**: If a target name argument is explicitly provided, `TargetSelector` searches `Project.targets`. If not found, `ErrorCode::targetNotFound` is returned.
2. **Default Target**: If no target argument is given and `project.defaultTarget` is set, that target is selected.
3. **Single Executable Auto-Selection**: If no target argument or `defaultTarget` is specified, Cippie inspects executable targets. If exactly one executable target exists, it is selected automatically.
4. **Ambiguity Error**: If multiple runnable targets exist and no target name is supplied, Cippie prints a deterministic error listing available runnable targets and instructions.

## Source Pattern Expansion & Normalization

`SourceScanner` resolves target source files using pattern expansion:

- Supported extensions: `.cpp`, `.cc`, `.cxx`.
- Recursive matching: `**` expands subdirectories recursively.
- Ignored directories: `.git`, `.cippie`, `build`, `cmake-build-*`, and hidden directories starting with `.`.
- Deduplication and Normalization: All discovered file paths are normalized (`path::lexically_normal()`), sorted deterministically, and deduplicated.

## Build Directories & Object Mapping

Artifacts are strictly isolated by target triple, configuration, and target name:

- **Object Files**:
  ```text
  .cippie/build/<target-triple>/debug/<target>/obj/<relative-source>.o
  ```
  Preserving the relative source path under `obj/` prevents collision when files in different directories share the same filename.

- **Executable Output**:
  ```text
  .cippie/build/<target-triple>/debug/<target>/bin/<target>
  ```

## Compiler & Process Execution

- **Toolchain Detection**: Checked in order: `CXX` environment variable, `clang++`, `g++`, `c++`.
- **Structured Execution**: `CompileCommand` and `LinkCommand` encapsulate structured arguments without string shell concatenation.
- **POSIX Process Execution**: `ProcessRunner` uses `fork`, `execvp`, and `waitpid` with EINTR handling, signal termination mapping (`128 + signal`), missing binary detection (`127`), and `_exit` in child processes.
