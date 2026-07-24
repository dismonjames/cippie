# Architecture

- `app`: orchestration and command dispatch.
- `cli`: command-line parsing.
- `config`: Cippiefile loading and validation.
- `project`: project and target models.
- `build`: source discovery, planning, compiling and linking.
- `process`: child-process creation and exit handling.
- `toolchain`: compiler and linker discovery.
- `package`: dependency resolution.
- `cache`: incremental-build metadata.
- `diagnostics`: logs and errors.
- `platform`: operating-system behavior.
- `util`: shared utilities.
