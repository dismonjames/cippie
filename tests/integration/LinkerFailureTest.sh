#!/usr/bin/env bash
set -Eeuo pipefail

CIPPIE_BINARY="${1:?missing cippie binary path}"

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "${TMP_DIR}"' EXIT

cd "${TMP_DIR}"

cat <<'EOF' > Cippiefile
project("bad-link") {
    cpp = 23;
    executable("bad-link") {
        entry = "src/main.cpp";
    }
}
EOF

mkdir -p src
cat <<'EOF' > src/main.cpp
void undefined_function();
int main() {
    undefined_function();
}
EOF

if "${CIPPIE_BINARY}" build; then
    echo "Expected build to fail due to linker error!"
    exit 1
fi
echo "Linker failure handled correctly"
