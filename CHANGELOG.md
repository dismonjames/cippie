# Changelog

## Unreleased

### Added

- Complete target system, project generation, test runner, and clean workflow (Milestones 3 & 6):
  - Multi-target build support: `executable`, `static_library` (`lib<name>.a`), `shared_library` (`lib<name>.so` with `-fPIC` and Linux RPATH), and `test`.
  - Target dependency graph (`BuildGraph`) with directed cycle detection, topological ordering, and build prevention on failed dependencies.
  - Transitive public include propagation (`publicIncludes` / `public_includes`) and compile definition propagation across linked target dependencies.
  - Reusable static and shared library outputs linked into dependent executables.
  - Runnable target selection (`TargetSelector`): rejecting library targets for `cippie run`, auto-selecting single executable targets, and reporting deterministic ambiguity errors when multiple executables exist without `defaultTarget`.
  - Test runner (`cippie test [target]`): building test targets, executing test binaries, printing concise test summaries, and returning exit code 7 on failure.
  - Safe clean command (`cippie clean`): canonicalizing paths, refusing root/system/project deletion, and avoiding symlink traversal outside project build directory.
  - Project generator (`cippie new <name>`): generating initial `Cippiefile`, `src/main.cpp`, `include/`, `tests/`, `README.md`, and `.gitignore` with project name validation and directory collision protection.
  - Ninja-formatted multi-target build output (`[X/N] CXX ...`, `[Y/N] AR ...`, `[Z/N] LINK ...`).
  - Unit and integration tests for build graph ordering, cycle detection, artifact naming, include propagation, clean safety, project generation, and multi-target builds.
- Real single-target build and run pipeline (Milestone 2).
- Complete Cippiefile frontend (Milestone 1).
- Initial project architecture and bootstrap CMake build.
