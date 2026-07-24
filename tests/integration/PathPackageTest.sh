#!/usr/bin/env bash
set -Eeuo pipefail

CIPPIE_BINARY="${1:?missing cippie binary path}"

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "${TMP_DIR}"' EXIT

cd "${TMP_DIR}"

mkdir -p local-lib/src local-lib/include main-app/src

cat <<'EOF' > local-lib/Cippiefile
project("local-lib") {
    cpp = 23;
    static_library("local-lib") {
        sources = ["src/**/*.cpp"];
        public_includes = ["include"];
    }
}
EOF

cat <<'EOF' > local-lib/include/locallib.hpp
#pragma once
int get_val();
EOF

cat <<'EOF' > local-lib/src/locallib.cpp
#include <locallib.hpp>
int get_val() { return 100; }
EOF

cat <<'EOF' > main-app/Cippiefile
project("main-app") {
    cpp = 23;
    dependencies = [
        pathPackage("local-lib", "../local-lib")
    ];
    executable("main-app") {
        entry = "src/main.cpp";
        links = ["local-lib"];
    }
}
EOF

cat <<'EOF' > main-app/src/main.cpp
#include <locallib.hpp>
#include <iostream>
int main() {
    std::cout << "Val: " << get_val() << "\n";
    return 0;
}
EOF

cd main-app
"${CIPPIE_BINARY}" restore
"${CIPPIE_BINARY}" build
"${CIPPIE_BINARY}" run main-app

echo "PathPackageTest passed successfully"
