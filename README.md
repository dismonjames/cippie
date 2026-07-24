# Cippie

Cippie is a C++ build system and package manager.

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
