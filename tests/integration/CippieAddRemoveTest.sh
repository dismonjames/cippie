#!/usr/bin/env bash
set -Eeuo pipefail

CIPPIE_BINARY="${1:?missing cippie binary path}"

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "${TMP_DIR}"' EXIT

cd "${TMP_DIR}"

REGISTRY_DIR="${TMP_DIR}/registry"
CACHE_DIR="${TMP_DIR}/cache"
export CIPPIE_REGISTRY_URL="file://${REGISTRY_DIR}"
export CIPPIE_CACHE_DIR="${CACHE_DIR}"

mkdir -p "${REGISTRY_DIR}/index/dummy" "${REGISTRY_DIR}/packages/dummy"

PKG_TMP="${TMP_DIR}/dummy-src"
mkdir -p "${PKG_TMP}/src" "${PKG_TMP}/include"

cat <<'EOF' > "${PKG_TMP}/Cippiefile"
project("dummy") {
    cpp = 23;
    static_library("dummy") {
        sources = ["src/**/*.cpp"];
    }
}
EOF

cat <<'EOF' > "${PKG_TMP}/src/dummy.cpp"
void dummy_func() {}
EOF

tar -czf "${REGISTRY_DIR}/packages/dummy/dummy-1.0.0.tar.gz" -C "${PKG_TMP}" .
SHA256_HASH=$(sha256sum "${REGISTRY_DIR}/packages/dummy/dummy-1.0.0.tar.gz" | awk '{print $1}')

echo "1.0.0" > "${REGISTRY_DIR}/index/dummy/versions.txt"
cat <<EOF > "${REGISTRY_DIR}/index/dummy/1.0.0.manifest"
version 1.0.0
sha256 ${SHA256_HASH}
url file://${REGISTRY_DIR}/packages/dummy/dummy-1.0.0.tar.gz
EOF

mkdir -p app/src
cd app

cat <<'EOF' > Cippiefile
project("app") {
    cpp = 23;
    executable("app") {
        entry = "src/main.cpp";
    }
}
EOF

cat <<'EOF' > src/main.cpp
int main() { return 0; }
EOF

# Test cippie add
"${CIPPIE_BINARY}" add dummy@^1.0.0
grep 'package("dummy"' Cippiefile

# Test cippie remove
"${CIPPIE_BINARY}" remove dummy
! grep 'package("dummy"' Cippiefile

echo "CippieAddRemoveTest passed successfully"
