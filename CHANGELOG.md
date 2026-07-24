# Changelog

## Unreleased

### Added

- Complete Cippiefile frontend (Milestone 1):
  - Token model with line/column/offset `SourceLocation` tracking.
  - Full lexer with UTF-8, escape sequence decoding, and comment skipping.
  - Recursive descent parser with intermediate AST and error synchronization.
  - Semantic validator producing strongly typed `Project` model.
  - Cycle detection for target dependencies.
  - Detailed diagnostic reporting and ANSI formatting (`DiagnosticPrinter`).
  - Real `ConfigLoader` parsing `Cippiefile` without hardcoded fallback model.
  - Comprehensive unit test suite (`ConfigLexerTests`, `ConfigParserTests`, `ConfigValidatorTests`).
  - Invalid configuration test fixtures.
- Initial project architecture and bootstrap CMake build.
- CLI, project discovery, source scanning, build planning, and process execution.
