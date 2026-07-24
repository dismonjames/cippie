#!/usr/bin/env bash
set -Eeuo pipefail

CIPPIE_BINARY="${1:?missing cippie binary path}"

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "${TMP_DIR}"' EXIT

cd "${TMP_DIR}"

cat <<'EOF' > Cippiefile
project("arg-test") {
    cpp = 23;
    executable("arg-test") {
        entry = "src/main.cpp";
    }
}
EOF

mkdir -p src
cat <<'EOF' > src/main.cpp
#include <iostream>
int main(int argc, char* argv[]) {
    if (argc < 3) return 1;
    if (std::string(argv[1]) != "--foo" || std::string(argv[2]) != "bar") {
        return 2;
    }
    std::cout << "Arg forwarding test passed\n";
    return 0;
}
EOF

"${CIPPIE_BINARY}" run -- --foo bar
