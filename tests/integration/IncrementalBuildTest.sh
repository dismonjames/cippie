#!/usr/bin/env bash
set -Eeuo pipefail

CIPPIE_BINARY="${1:?missing cippie binary path}"

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "${TMP_DIR}"' EXIT

cd "${TMP_DIR}"

cat <<'EOF' > Cippiefile
project("inc-test") {
    cpp = 23;
    executable("inc-app") {
        entry = "src/main.cpp";
        sources = ["src/**/*.cpp"];
        includes = ["include"];
    }
}
EOF

mkdir -p src include

cat <<'EOF' > include/header.hpp
#pragma once
int get_num();
EOF

cat <<'EOF' > src/func.cpp
#include <header.hpp>
int get_num() { return 42; }
EOF

cat <<'EOF' > src/main.cpp
#include <header.hpp>
#include <iostream>
int main() {
    std::cout << get_num() << "\n";
    return 0;
}
EOF

# 1. First build compiles all sources
OUT1="$("${CIPPIE_BINARY}" build -v)"
echo "${OUT1}" | grep "CXX src/func.cpp"
echo "${OUT1}" | grep "CXX src/main.cpp"

# 2. Second unchanged build skips compile steps (CACHED)
OUT2="$("${CIPPIE_BINARY}" build -v)"
echo "${OUT2}" | grep "CACHED src/func.cpp"
echo "${OUT2}" | grep "CACHED src/main.cpp"

# 3. Touch source without content change -> still CACHED
touch src/func.cpp
OUT3="$("${CIPPIE_BINARY}" build -v)"
echo "${OUT3}" | grep "CACHED src/func.cpp"

# 4. Modify header content -> triggers recompilation of including sources
cat <<'EOF' > include/header.hpp
#pragma once
int get_num(); // modified comment
EOF

OUT4="$("${CIPPIE_BINARY}" build -v)"
echo "${OUT4}" | grep "CXX src/func.cpp"
echo "${OUT4}" | grep "CXX src/main.cpp"

echo "IncrementalBuildTest passed successfully"
