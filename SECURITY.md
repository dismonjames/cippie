# Security Policy & Boundaries

## Security Model

Cippie executes compiler toolchains (`clang++`, `g++`, `ar`) and user programs directly with the privileges of the executing user.

### Security Guarantees
- **No Shell String Concatenation**: Process execution uses structured argument arrays (`execv` style) without passing user strings to `/bin/sh -c` or `std::system`.
- **Path Traversal Protection**: Archive extraction and package resolution reject path escaping outside target directories (`..` traversal attempts).
- **Process Isolation**: Toolchain commands and test executions run with structured arguments and clean environment isolation where appropriate.
- **Cache Isolation**: Build artifacts are isolated under `.cippie/build/<triple>/<config>/` to prevent cross-target or cross-configuration contamination.

### Security Limitations
Cippie is a build system. Compiling untrusted source code or running untrusted test executables executes arbitrary machine code with the current user's permissions. Always inspect untrusted source code or packages before building them.

## Reporting a Vulnerability

Please report security issues directly to the repository maintainers.
