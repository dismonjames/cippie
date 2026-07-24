#!/usr/bin/env bash
set -Eeuo pipefail

CIPPIE_BINARY="${1:?missing cippie binary path}"

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "${TMP_DIR}"' EXIT

cd "${TMP_DIR}"

cat <<'EOF' > Cippiefile
project("static-lib-app") {
    cpp = 23;

    static_library("math") {
        sources = ["math/src/**/*.cpp"];
        public_includes = ["math/include"];
    }

    executable("calculator") {
        entry = "app/src/main.cpp";
        sources = ["app/src/**/*.cpp"];
        links = ["math"];
    }
}
EOF

mkdir -p math/include math/src app/src

cat <<'EOF' > math/include/add.hpp
#pragma once
int add(int a, int b);
EOF

cat <<'EOF' > math/src/add.cpp
#include <add.hpp>
int add(int a, int b) {
    return a + b;
}
EOF

cat <<'EOF' > app/src/main.cpp
#include <add.hpp>
#include <iostream>
int main() {
    std::cout << "Sum: " << add(2, 3) << "\n";
    return 0;
}
EOF

"${CIPPIE_BINARY}" build calculator
"${CIPPIE_BINARY}" run calculator
