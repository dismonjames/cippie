# Cippiefile v0.1 Specification & Schema

This document defines the canonical specification for `Cippiefile` schema v0.1.0.

## 1. Structure Overview

A `Cippiefile` uses a declarative syntax inspired by C-style object declarations.

```cpp
project("my_project") {
    cpp = 23;
    defaultTarget = "app";

    static_library("core") {
        sources = ["src/core/**/*.cpp"];
        includes = ["include"];
        public_includes = ["include"];
        defines = ["ENABLE_CORE=1"];
    }

    executable("app") {
        entry = "src/main.cpp";
        sources = ["src/app/**/*.cpp"];
        links = ["core"];
    }

    test("core_test") {
        entry = "tests/test_core.cpp";
        links = ["core"];
    }
}
```

## 2. Top-Level Project Fields

| Field | Type | Description | Required | Default |
|---|---|---|---|---|
| `name` | string | Name of the project | Yes | Specified in `project("name")` |
| `cpp` | integer | C++ standard (`17`, `20`, `23`) | No | `23` |
| `defaultTarget` | string | Name of the default build target | No | Automatically inferred if 1 target exists |
| `configurations` | block | Custom build configurations | No | `debug` and `release` |
| `dependencies` | array | Array of package dependency specifications | No | `[]` |

## 3. Target Types & Fields

Supported target declarations:
- `executable("name")`: Executable program. Must specify `entry`.
- `static_library("name")`: Static library archive (`.a`).
- `shared_library("name")`: Dynamic shared library (`.so`).
- `test("name")`: Automated test executable target.

| Field | Type | Description | Canonical / Aliases |
|---|---|---|---|
| `entry` | string | Path to entry source file (`main()`) | Required for executables/tests |
| `sources` | array[string] | Glob pattern array for source matching | `["src/**/*.cpp"]` |
| `includes` | array[string] | Private header search paths | Private to target |
| `public_includes` | array[string] | Public header search paths propagated to linked targets | Alias: `publicIncludes` |
| `defines` | array[string] | Preprocessor macros | e.g. `["FOO=1"]` |
| `compileOptions` | array[string] | Additional compiler flags | e.g. `["-Wall", "-O3"]` |
| `linkOptions` | array[string] | Additional linker flags | e.g. `["-pthread"]` |
| `links` | array[string] | Targets or packages to link | Alias: `dependencies` |

## 4. Dependencies

Package dependencies can be specified in top-level `dependencies`:
- `package("pkg_name", "1.0.0")`: Static or remote registry package.
- `path_package("pkg_name", "../path")`: Local path dependency. (Alias: `pathPackage`).
- `git_package("pkg_name", "https://github.com/...", tag = "v1.0")`: Git repository dependency. (Alias: `gitPackage`).
