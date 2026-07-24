#!/usr/bin/env bash
set -Eeuo pipefail

CIPPIE_BINARY="${1:?missing cippie binary path}"

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "${TMP_DIR}"' EXIT

cd "${TMP_DIR}"

cat <<'EOF' > Cippiefile
project("single-exec") {
    cpp = 23;
    executable("single-exec") {
        entry = "src/main.cpp";
        sources = ["src/**/*.cpp"];
    }
}
EOF

mkdir -p src
cat <<'EOF' > src/main.cpp
#include <iostream>
void helper();
int main() {
    helper();
    std::cout << "OK single exec\n";
    return 0;
}
EOF

cat <<'EOF' > src/helper.cpp
#include <iostream>
void helper() {
    std::cout << "helper function\n";
}
EOF

"${CIPPIE_BINARY}" build
"${CIPPIE_BINARY}" run
