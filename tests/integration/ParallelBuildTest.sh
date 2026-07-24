#!/usr/bin/env bash
set -Eeuo pipefail

CIPPIE_BINARY="${1:?missing cippie binary path}"

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "${TMP_DIR}"' EXIT

cd "${TMP_DIR}"

cat <<'EOF' > Cippiefile
project("parallel-test") {
    cpp = 23;
    executable("parallel-app") {
        entry = "src/main.cpp";
        sources = ["src/**/*.cpp"];
    }
}
EOF

mkdir -p src

cat <<'EOF' > src/main.cpp
#include <iostream>
int main() { std::cout << "Parallel app OK\n"; return 0; }
EOF

cat <<'EOF' > src/a.cpp
void func_a() {}
EOF

cat <<'EOF' > src/b.cpp
void func_b() {}
EOF

# 1. Test --jobs 1
"${CIPPIE_BINARY}" clean --all
"${CIPPIE_BINARY}" build --jobs 1
"${CIPPIE_BINARY}" run parallel-app

# 2. Test --jobs 4
"${CIPPIE_BINARY}" clean --all
"${CIPPIE_BINARY}" build --jobs 4
"${CIPPIE_BINARY}" run parallel-app

echo "ParallelBuildTest passed successfully"
