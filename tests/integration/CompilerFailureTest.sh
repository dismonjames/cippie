#!/usr/bin/env bash
set -Eeuo pipefail

CIPPIE_BINARY="${1:?missing cippie binary path}"

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "${TMP_DIR}"' EXIT

cd "${TMP_DIR}"

cat <<'EOF' > Cippiefile
project("bad-compile") {
    cpp = 23;
    executable("bad-compile") {
        entry = "src/main.cpp";
    }
}
EOF

mkdir -p src
cat <<'EOF' > src/main.cpp
int main() {
    this_is_a_syntax_error!
}
EOF

if "${CIPPIE_BINARY}" build; then
    echo "Expected build to fail due to syntax error!"
    exit 1
fi
echo "Compiler failure handled correctly"
