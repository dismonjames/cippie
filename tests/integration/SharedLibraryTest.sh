#!/usr/bin/env bash
set -Eeuo pipefail

CIPPIE_BINARY="${1:?missing cippie binary path}"

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "${TMP_DIR}"' EXIT

cd "${TMP_DIR}"

cat <<'EOF' > Cippiefile
project("shared-lib-app") {
    cpp = 23;

    shared_library("greeter") {
        sources = ["greeter/src/**/*.cpp"];
        public_includes = ["greeter/include"];
    }

    executable("hello") {
        entry = "app/src/main.cpp";
        sources = ["app/src/**/*.cpp"];
        links = ["greeter"];
    }
}
EOF

mkdir -p greeter/include greeter/src app/src

cat <<'EOF' > greeter/include/greet.hpp
#pragma once
const char* get_greeting();
EOF

cat <<'EOF' > greeter/src/greet.cpp
#include <greet.hpp>
const char* get_greeting() {
    return "Hello Shared World!";
}
EOF

cat <<'EOF' > app/src/main.cpp
#include <greet.hpp>
#include <iostream>
int main() {
    std::cout << get_greeting() << "\n";
    return 0;
}
EOF

"${CIPPIE_BINARY}" build hello
"${CIPPIE_BINARY}" run hello
