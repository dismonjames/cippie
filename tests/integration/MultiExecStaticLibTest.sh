#!/usr/bin/env bash
set -Eeuo pipefail

CIPPIE_BINARY="${1:?missing cippie binary path}"

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "${TMP_DIR}"' EXIT

cd "${TMP_DIR}"

cat <<'EOF' > Cippiefile
project("multi-exec-static-lib") {
    cpp = 23;

    static_library("common") {
        sources = ["common/src/**/*.cpp"];
        public_includes = ["common/include"];
    }

    executable("client") {
        entry = "client/src/main.cpp";
        sources = ["client/src/**/*.cpp"];
        links = ["common"];
    }

    executable("server") {
        entry = "server/src/main.cpp";
        sources = ["server/src/**/*.cpp"];
        links = ["common"];
    }
}
EOF

mkdir -p common/include common/src client/src server/src

cat <<'EOF' > common/include/msg.hpp
#pragma once
const char* get_message();
EOF

cat <<'EOF' > common/src/msg.cpp
#include <msg.hpp>
const char* get_message() { return "Shared Common Lib"; }
EOF

cat <<'EOF' > client/src/main.cpp
#include <msg.hpp>
#include <iostream>
int main() { std::cout << "Client: " << get_message() << "\n"; return 0; }
EOF

cat <<'EOF' > server/src/main.cpp
#include <msg.hpp>
#include <iostream>
int main() { std::cout << "Server: " << get_message() << "\n"; return 0; }
EOF

"${CIPPIE_BINARY}" build client
"${CIPPIE_BINARY}" run client
"${CIPPIE_BINARY}" build server
"${CIPPIE_BINARY}" run server
