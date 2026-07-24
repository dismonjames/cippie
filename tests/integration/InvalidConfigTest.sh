#!/usr/bin/env bash
set -Eeuo pipefail

CIPPIE_BINARY="${1:?missing cippie binary path}"

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "${TMP_DIR}"' EXIT

cd "${TMP_DIR}"

cat <<'EOF' > Cippiefile
project("ambiguous") {
    cpp = 23;
    executable("client") {
        entry = "src/client.cpp";
        sources = ["src/client.cpp"];
    }
    executable("server") {
        entry = "src/server.cpp";
        sources = ["src/server.cpp"];
    }
}
EOF

mkdir -p src
echo "int main(){}" > src/client.cpp
echo "int main(){}" > src/server.cpp

if "${CIPPIE_BINARY}" run; then
    echo "Expected cippie run to fail due to ambiguous target selection!"
    exit 1
fi

"${CIPPIE_BINARY}" run client
"${CIPPIE_BINARY}" run server
