#!/usr/bin/env bash
set -Eeuo pipefail

CIPPIE_BINARY="${1:?missing cippie binary path}"

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "${TMP_DIR}"' EXIT

cd "${TMP_DIR}"

# 1. Create project
"${CIPPIE_BINARY}" new my_app

cd my_app
test -f Cippiefile
test -f src/main.cpp
test -d include
test -d tests
test -f README.md
test -f .gitignore

# 2. Build and run generated project
"${CIPPIE_BINARY}" run

cd "${TMP_DIR}"

# 3. Refuse overwriting non-empty directory
if "${CIPPIE_BINARY}" new my_app; then
    echo "Expected cippie new to fail on existing non-empty directory!"
    exit 1
fi

# 4. Refuse invalid project name
if "${CIPPIE_BINARY}" new "invalid name!"; then
    echo "Expected cippie new to fail on invalid project name!"
    exit 1
fi

echo "ProjectNewTest passed successfully"
