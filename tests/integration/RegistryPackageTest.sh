#!/usr/bin/env bash
set -Eeuo pipefail

CIPPIE_BINARY="${1:?missing cippie binary path}"

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "${TMP_DIR}"' EXIT

cd "${TMP_DIR}"

# Set up local static registry fixture
REGISTRY_DIR="${TMP_DIR}/registry"
CACHE_DIR="${TMP_DIR}/cache"
export CIPPIE_REGISTRY_URL="file://${REGISTRY_DIR}"
export CIPPIE_CACHE_DIR="${CACHE_DIR}"

mkdir -p "${REGISTRY_DIR}/index/myfmt" "${REGISTRY_DIR}/packages/myfmt"

# Create myfmt 1.0.0 source package tarball
PKG_TMP="${TMP_DIR}/pkg-src"
mkdir -p "${PKG_TMP}/src" "${PKG_TMP}/include"

cat <<'EOF' > "${PKG_TMP}/Cippiefile"
project("myfmt") {
    cpp = 23;
    static_library("myfmt") {
        sources = ["src/**/*.cpp"];
        public_includes = ["include"];
    }
}
EOF

cat <<'EOF' > "${PKG_TMP}/include/myfmt.hpp"
#pragma once
const char* get_fmt();
EOF

cat <<'EOF' > "${PKG_TMP}/src/myfmt.cpp"
#include <myfmt.hpp>
const char* get_fmt() { return "myfmt v1.0.0 OK"; }
EOF

tar -czf "${REGISTRY_DIR}/packages/myfmt/myfmt-1.0.0.tar.gz" -C "${PKG_TMP}" .

SHA256_HASH=$(sha256sum "${REGISTRY_DIR}/packages/myfmt/myfmt-1.0.0.tar.gz" | awk '{print $1}')

echo "1.0.0" > "${REGISTRY_DIR}/index/myfmt/versions.txt"
cat <<EOF > "${REGISTRY_DIR}/index/myfmt/1.0.0.manifest"
version 1.0.0
sha256 ${SHA256_HASH}
url file://${REGISTRY_DIR}/packages/myfmt/myfmt-1.0.0.tar.gz
EOF

# Create consuming app
mkdir -p app/src
cd app

cat <<'EOF' > Cippiefile
project("app") {
    cpp = 23;
    dependencies = [
        package("myfmt", "^1.0.0")
    ];
    executable("app") {
        entry = "src/main.cpp";
        links = ["myfmt"];
    }
}
EOF

cat <<'EOF' > src/main.cpp
#include <myfmt.hpp>
#include <iostream>
int main() {
    std::cout << get_fmt() << "\n";
    return 0;
}
EOF

"${CIPPIE_BINARY}" restore
"${CIPPIE_BINARY}" build
"${CIPPIE_BINARY}" run app

echo "RegistryPackageTest passed successfully"
