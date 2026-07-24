# Cippie

Cippie is a modern C++23 build system, project manager, and package manager.

## Usage

```bash
# Build the default or specified target
cippie build [target]

# Build and run target, forwarding arguments after --
cippie run [target] [-- program arguments...]
```

## Bootstrap build

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

cmake --build build
ctest --test-dir build --output-on-failure
```

## License

GNU General Public License version 3 or later.
