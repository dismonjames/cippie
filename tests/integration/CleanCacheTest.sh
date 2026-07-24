#!/usr/bin/env bash
set -Eeuo pipefail

CIPPIE_BINARY="${1:?missing cippie binary path}"

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "${TMP_DIR}"' EXIT

cd "${TMP_DIR}"

cat <<'EOF' > Cippiefile
project("clean-cache-test") {
    cpp = 23;
    executable("app") {
        entry = "src/main.cpp";
    }
}
EOF

mkdir -p src
cat <<'EOF' > src/main.cpp
int main() { return 0; }
EOF

"${CIPPIE_BINARY}" build -v

# Clean cache
"${CIPPIE_BINARY}" clean --cache

# Next build must perform full re-compilation (not CACHED)
OUT="$("${CIPPIE_BINARY}" build -v)"
echo "${OUT}" | grep "CXX src/main.cpp"

echo "CleanCacheTest passed successfully"
