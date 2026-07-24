#!/usr/bin/env bash
set -Eeuo pipefail

CIPPIE_BINARY="${1:?missing cippie binary path}"

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "${TMP_DIR}"' EXIT

cd "${TMP_DIR}"

cat <<'EOF' > Cippiefile
project("test-suite") {
    cpp = 23;

    test("unit_test") {
        entry = "tests/unit.cpp";
        sources = ["tests/unit.cpp"];
    }
}
EOF

mkdir -p tests
cat <<'EOF' > tests/unit.cpp
#include <iostream>
int main() {
    std::cout << "Unit test passed!\n";
    return 0;
}
EOF

"${CIPPIE_BINARY}" test

# Now update unit.cpp to fail
cat <<'EOF' > tests/unit.cpp
#include <iostream>
int main() {
    std::cout << "Unit test failed!\n";
    return 1;
}
EOF

if "${CIPPIE_BINARY}" test; then
    echo "Expected cippie test to fail on non-zero exit code!"
    exit 1
fi

echo "Test target handling verified successfully"
