# Cippie

Cippie is a modern C++23 build system, project manager, and package manager.

## Usage

```bash
# Create a new project
cippie new <name>

# Build the default or specified target
cippie build [target]

# Build and run an executable target, forwarding arguments after --
cippie run [target] [-- program arguments...]

# Build and run test targets
cippie test [target]

# Remove generated build files (.cippie/build)
cippie clean
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
