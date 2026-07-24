# Cippiefile

`Cippiefile` is a custom declarative DSL for defining Cippie projects.

## Grammar & Syntax

```cpp
project("MyProject") {
    cpp = 23;
    defaultTarget = "client";

    configurations = {
        debug {
            optimization = 0;
            debugSymbols = true;
            warningsAsErrors = false;
        }

        release {
            optimization = 3;
            debugSymbols = false;
            lto = true;
        }
    };

    dependencies = [
        package("fmt", "11.2.0"),
        package("nlohmann-json", "^3.12.0")
    ];

    static_library("core") {
        sources = [
            "src/core/**/*.cpp"
        ];

        includes = [
            "src/core"
        ];

        publicIncludes = [
            "include"
        ];

        defines = [
            "MY_SUITE_CORE"
        ];
    }

    shared_library("util") {
        sources = [
            "src/util/**/*.cpp"
        ];

        publicIncludes = [
            "include"
        ];
    }

    executable("client") {
        entry = "apps/client/main.cpp";

        sources = [
            "apps/client/**/*.cpp"
        ];

        links = [
            "core",
            "util",
            dependency("fmt")
        ];
    }

    test("unit_tests") {
        entry = "tests/unit_main.cpp";

        sources = [
            "tests/**/*.cpp"
        ];

        links = [
            "core"
        ];
    }
}
```

## Supported Target Declaration Kinds

- `executable("<name>") { ... }`: Executable binary target.
- `static_library("<name>") { ... }`: Static library target (`lib<name>.a`).
- `shared_library("<name>") { ... }`: Shared library target (`lib<name>.so`).
- `library("<name>") { type = "static" | "shared"; ... }`: Generic library target block.
- `test("<name>") { ... }`: Test executable target executed by `cippie test`.

## Frontend Pipeline

1. **Lexer**: Tokenizes source into tokens with line, column, and byte offset tracking. Supports string escape sequences, UTF-8, line comments (`//`), and block comments (`/* ... */`).
2. **Parser**: Recursive descent parser building an explicit intermediate AST (`AstProjectDeclaration`, `AstTargetDeclaration`, `AstValue`, etc.) with panic-mode error recovery.
3. **Validator**: Converts AST to strongly-typed `Project` model, enforcing semantic rules:
   - Unique target and dependency names.
   - Enforces `entry` requirement for executables.
   - Enforces explicit `type` (`static` or `shared`) for libraries and forbids `entry`.
   - Prevents source paths escaping project root (`..`).
   - Detects target dependency cycles.
