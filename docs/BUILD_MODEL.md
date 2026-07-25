# Cippie Build Model

This document specifies the build pipeline, target selection logic, object path layout, dependency graph, and execution model implemented in Cippie.

## Target Selection Pipeline

### `cippie build [target]`
1. **Explicit Target**: If a target name argument is explicitly provided, `TargetSelector` searches `Project.targets`.
2. **Default Target**: If no target argument is given and `project.defaultTarget` is set, that target is selected.
3. **Single Target Auto-Selection**: If no target argument or `defaultTarget` is specified and only one buildable target exists, it is selected automatically.
4. **Ambiguity Error**: If multiple targets exist and none is specified, Cippie reports available build targets.

### `cippie run [target]`
1. **Explicit Target**: Must be an executable target (`TargetType::executable`). If a library target (`staticLibrary` or `sharedLibrary`) is specified, Cippie rejects execution with error: `"target '<name>' is a library and cannot be run"`.
2. **Default Target**: Must be an executable target.
3. **Single Executable Auto-Selection**: If exactly one executable target exists, it is selected automatically.
4. **Ambiguity Error**: If multiple executable targets exist and no `defaultTarget` is defined, Cippie reports a deterministic ambiguity error listing runnable targets. Libraries are never treated as runnable targets.

### `cippie test [target]`
1. If a test target is specified, builds and executes it.
2. If no target is specified, selects and runs all targets with `type == TargetType::test` (or executables if test targets are absent).
3. Prints a concise test summary and returns exit code `7` if any test target fails.

## Target Dependency Graph (`BuildGraph`)

- **Graph Structure**: Each target is a graph node. Links create directed dependencies.
- **Cycle Detection**: Cycles in target links are detected before compilation and reported with exact cycle paths (`ErrorCode::dependencyCycle`).
- **Topological Sorting**: Builds dependencies before dependents.
- **Include Propagation**: `publicIncludeDirectories` (and `public_includes`) of dependency targets propagate transitively to dependent compile commands.
- **Link Line Ordering**: Dependent executables and shared libraries automatically link static archives (`lib<name>.a`) and shared libraries (`lib<name>.so`). For Linux shared libraries, RPATH flags (`-Wl,-rpath,<libdir>`) are automatically injected.

## Source Pattern Expansion & Normalization

`SourceScanner` resolves target source files using pattern expansion:

- Supported extensions: `.cpp`, `.cc`, `.cxx`.
- Recursive matching: `**` expands subdirectories recursively.
- Ignored directories: `.git`, `.cippie`, `build`, `cmake-build-*`, and hidden directories starting with `.`.
- Deduplication and Normalization: All discovered file paths are normalized (`path::lexically_normal()`), sorted deterministically, and deduplicated.

## Build Directories & Artifact Layout

Artifacts are strictly isolated by target triple, configuration, and target name:

- **Object Files**:
  ```text
  .cippie/build/<target-triple>/debug/<target>/obj/<relative-source>.o
  ```
- **Static Library**:
  ```text
  .cippie/build/<target-triple>/debug/<target>/lib/lib<target>.a
  ```
- **Shared Library**:
  ```text
  .cippie/build/<target-triple>/debug/<target>/lib/lib<target>.so
  ```
- **Executable Binary**:
  ```text
  .cippie/build/<target-triple>/debug/<target>/bin/<target>
  ```

## Safe Clean (`cippie clean`)

`CleanRunner` removes `.cippie/build` while enforcing deletion safety:
- Refuses deleting system paths (`/`, `/home`, `~`) or project root.
- Ensures target is strictly inside `.cippie/build`.
- Does not follow symlinks outside project boundary.

## Project Creation (`cippie new <name>`)

`ProjectGenerator` initializes a ready-to-build Cippie project:
- Validates project name (ASCII alphanumerics, `_`, `-`, no spaces or illegal characters).
- Refuses overwriting non-empty existing directories.
- Creates `Cippiefile`, `src/main.cpp`, `include/`, `tests/`, `README.md`, `.gitignore`.
- Immediately buildable and runnable via `cippie run`.

## Self-Hosting Development Pipeline

Cippie compiles itself using its own root `Cippiefile`:
- Production targets: `cippie_core` (static library) and `cippie` (executable).
- Development commands:
  - `cippie build`: Builds Cippie using Cippie.
  - `cippie test`: Runs Cippie unit & integration tests.
  - `./scripts/self-host.sh`: Runs 2-generation verification (Gen1 → Gen2).
- CMake is retained exclusively as a temporary bootstrap fallback for initial checkouts.
