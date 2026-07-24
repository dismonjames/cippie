#!/usr/bin/env bash
set -Eeuo pipefail

CIPPIE_BINARY="${1:?missing cippie binary path}"

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "${TMP_DIR}"' EXIT

cd "${TMP_DIR}"

cat <<'EOF' > Cippiefile
project("clean-test") {
    cpp = 23;
    executable("clean-test") {
        entry = "src/main.cpp";
    }
}
EOF

mkdir -p src
cat <<'EOF' > src/main.cpp
int main() { return 0; }
EOF

"${CIPPIE_BINARY}" build
test -d .cippie/build

"${CIPPIE_BINARY}" clean
test ! -d .cippie/build
test -f Cippiefile
test -f src/main.cpp

"${CIPPIE_BINARY}" run
test -d .cippie/build

echo "CleanTest passed successfully"
