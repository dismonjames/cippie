# Contributing to Cippie

Cippie uses Cippie itself as its primary development build system.

## Developer Workflow

```bash
# Primary build command
cippie build

# Run project test suite
cippie test

# Run repository quality & self-host check
./scripts/check.sh
```

## Bootstrap Fallback

If Cippie is not yet installed on your system:

```bash
./scripts/bootstrap.sh
```

CMake is used strictly as a bootstrap compiler to build the initial Cippie binary.

## Guidelines

- Keep each source file focused on a single responsibility.
- Preserve C++23 standards compliance.
- Run `./scripts/check.sh` before submitting pull requests.
- Do not commit build output directories (`build/`, `.cippie/`, `dist/`), logs, or temporary files.
