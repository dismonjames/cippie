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

    library("core") {
        type = static;

        sources = [
            "src/core/**/*.cpp"
        ];

        includes = [
            "include"
        ];

        publicIncludes = [
            "include"
        ];

        defines = [
            "MY_SUITE_CORE"
        ];
    }

    executable("client") {
        entry = "apps/client/main.cpp";

        sources = [
            "apps/client/**/*.cpp"
        ];

        links = [
            "core",
            dependency("fmt")
        ];
    }
}
```

## Frontend Pipeline

1. **Lexer**: Tokenizes source into tokens with line, column, and byte offset tracking. Supports string escape sequences, UTF-8, line comments (`//`), and block comments (`/* ... */`).
2. **Parser**: Recursive descent parser building an explicit intermediate AST (`AstProjectDeclaration`, `AstTargetDeclaration`, `AstValue`, etc.) with panic-mode error recovery.
3. **Validator**: Converts AST to strongly-typed `Project` model, enforcing semantic rules:
   - Unique target and dependency names.
   - Enforces `entry` requirement for executables.
   - Enforces explicit `type` (`static` or `shared`) for libraries and forbids `entry`.
   - Prevents source paths escaping project root (`..`).
   - Detects target dependency cycles.
