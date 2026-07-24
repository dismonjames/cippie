# Changelog

## Unreleased

### Added

- Real single-target build and run pipeline (Milestone 2):
  - Target selection logic (`TargetSelector`) supporting explicit targets, `defaultTarget`, auto-selection for single executables, and deterministic error reporting on ambiguous targets.
  - Deterministic source pattern scanner (`SourceScanner`) supporting `*.cpp`, `*.cc`, `*.cxx`, recursive `**` matching, directory filtering (`.git`, `.cippie`, `build`, `cmake-build-*`), normalization, sorting, and deduplication.
  - Linux toolchain detector (`ToolchainDetector`) supporting `CXX`, `clang++`, `g++`, `c++`, compiler version detection, and `TargetTriple` (`x86_64-linux-gnu`).
  - Structured compile & link commands (`CompileCommand`, `LinkCommand`) and build planner (`BuildPlanner`) generating collision-free object paths under `.cippie/build/<triple>/debug/<target>/obj/<relative-source>.o`.
  - POSIX process runner (`LinuxProcessRunner`) with `fork`, `execvp`, `waitpid` EINTR retries, signal termination mapping, missing executable detection (`127`), and permission denied handling (`126`).
  - Ninja-style build progress logging (`[1/N] CXX ...`, `[N/N] LINK ...`).
  - `cippie build [target]` and `cippie run [target] [-- program arguments...]` forwarding program arguments after `--` and propagating child exit codes.
  - Expanded unit and integration test suite (`TargetSelectorTests`, `BuildPlannerTests`, `ProcessRunnerTests`, `BuildSingleExecutableTest.sh`, `RunArgumentsTest.sh`, `CompilerFailureTest.sh`, `LinkerFailureTest.sh`, `SubdirectoryRunTest.sh`, `InvalidConfigTest.sh`).
- Complete Cippiefile frontend (Milestone 1).
- Initial project architecture and bootstrap CMake build.
