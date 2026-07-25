#!/usr/bin/env bash
# Cippie Primary Test Wrapper Script
set -Eeuo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

CIPPIE_EXEC=""

if command -v cippie >/dev/null 2>&1; then
    CIPPIE_EXEC="cippie"
elif [[ -x "${ROOT_DIR}/build/cippie" ]]; then
    CIPPIE_EXEC="${ROOT_DIR}/build/cippie"
fi

if [[ -n "$CIPPIE_EXEC" ]]; then
    cd "${ROOT_DIR}"
    echo "Running Cippie test targets..."
    "${CIPPIE_EXEC}" test "$@"
else
    echo "Cippie binary not found. Running CTest test suite..."
    ctest --test-dir "${ROOT_DIR}/build" --output-on-failure "$@"
fi
