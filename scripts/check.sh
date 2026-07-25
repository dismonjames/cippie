#!/usr/bin/env bash
# Cippie Repository Comprehensive Quality Check
set -Eeuo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

cd "${ROOT_DIR}"

echo "=== 1. Validating shell scripts syntax ==="
bash -n scripts/*.sh
echo "Shell scripts syntax OK."

if command -v shellcheck >/dev/null 2>&1; then
    echo "Running ShellCheck..."
    shellcheck scripts/*.sh || echo "WARNING: ShellCheck reported issues"
else
    echo "Note: ShellCheck is not installed (optional)."
fi

echo ""
echo "=== 2. Running Source Synchronization Check ==="
python3 "${SCRIPT_DIR}/check-source-sync.py"

echo ""
echo "=== 3. Building Cippie Bootstrap ==="
"${SCRIPT_DIR}/build.sh" --bootstrap

echo ""
echo "=== 4. Running CTest Test Suite ==="
ctest --test-dir build --output-on-failure

echo ""
echo "=== 5. Running 2-Generation Self-Host Verification ==="
"${SCRIPT_DIR}/self-host.sh" --bootstrap "${ROOT_DIR}/build/cippie"

echo ""
echo "=== All Cippie Quality Checks Passed Successfully! ==="
