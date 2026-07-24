#!/usr/bin/env bash
set -Eeuo pipefail

CIPPIE_BINARY="${1:?missing cippie binary path}"

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "${TMP_DIR}"' EXIT

cd "${TMP_DIR}"

cat <<'EOF' > Cippiefile
project("sub-dir-test") {
    cpp = 23;
    executable("sub-dir-test") {
        entry = "src/main.cpp";
    }
}
EOF

mkdir -p src/nested/deep
cat <<'EOF' > src/main.cpp
#include <iostream>
int main() {
    std::cout << "Subdirectory build success\n";
    return 0;
}
EOF

cd src/nested/deep
"${CIPPIE_BINARY}" build
"${CIPPIE_BINARY}" run
