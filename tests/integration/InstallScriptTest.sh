#!/usr/bin/env bash
# Integration test suite for scripts/install.sh and scripts/uninstall.sh
set -Eeuo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"

INSTALL_SCRIPT="${ROOT_DIR}/scripts/install.sh"
UNINSTALL_SCRIPT="${ROOT_DIR}/scripts/uninstall.sh"
PACKAGE_SCRIPT="${ROOT_DIR}/scripts/package-release.sh"

echo "=== Running Installer & Uninstaller Integration Tests ==="

VERSION="0.1.6"

# 1. Package release tarball locally
(cd "${ROOT_DIR}" && "${PACKAGE_SCRIPT}")

DIST_DIR="${ROOT_DIR}/dist"
TARBALL="$(find "${DIST_DIR}" -name "cippie-${VERSION}-*.tar.gz" | head -1)"
CHECKSUM="$(find "${DIST_DIR}" -name "cippie-${VERSION}-*.tar.gz.sha256" | head -1)"

if [[ ! -f "$TARBALL" || ! -f "$CHECKSUM" ]]; then
    echo "ERROR: Release artifacts missing in dist/" >&2
    exit 1
fi

TMP_SERVER_DIR="$(mktemp -d)"
TMP_WORK_DIR="$(mktemp -d)"
trap 'rm -rf "${TMP_SERVER_DIR}" "${TMP_WORK_DIR}"' EXIT

cp "$TARBALL" "$CHECKSUM" "${TMP_SERVER_DIR}/"

LOCAL_BASE_URL="file://${TMP_SERVER_DIR}"

echo "Test 1: Install to explicit prefix with local file:// URL"
TEST_PREFIX="${TMP_WORK_DIR}/local_prefix"
CIPPIE_RELEASE_BASE_URL="${LOCAL_BASE_URL}" "${INSTALL_SCRIPT}" \
    --version "${VERSION}" \
    --prefix "${TEST_PREFIX}"

test -f "${TEST_PREFIX}/bin/cippie"
"${TEST_PREFIX}/bin/cippie" version | grep -q "${VERSION}"
echo "Test 1 PASSED."

echo "Test 2: Install to path containing spaces"
SPACE_DIR="${TMP_WORK_DIR}/dir with spaces/bin"
CIPPIE_RELEASE_BASE_URL="${LOCAL_BASE_URL}" "${INSTALL_SCRIPT}" \
    --version "${VERSION}" \
    --install-dir "${SPACE_DIR}"

test -f "${SPACE_DIR}/cippie"
"${SPACE_DIR}/cippie" version | grep -q "${VERSION}"
echo "Test 2 PASSED."

echo "Test 3: Existing binary without --force should fail"
if CIPPIE_RELEASE_BASE_URL="${LOCAL_BASE_URL}" "${INSTALL_SCRIPT}" \
    --version "${VERSION}" \
    --prefix "${TEST_PREFIX}" 2>/dev/null; then
    # Note: re-installing same version reports already installed (exit 0)
    :
fi

# Modify binary to simulate different version
echo "echo 'Cippie 0.0.9'" > "${TEST_PREFIX}/bin/cippie"
chmod +x "${TEST_PREFIX}/bin/cippie"

if CIPPIE_RELEASE_BASE_URL="${LOCAL_BASE_URL}" "${INSTALL_SCRIPT}" \
    --version "${VERSION}" \
    --prefix "${TEST_PREFIX}" 2>/dev/null; then
    echo "ERROR: Install without --force should have failed for conflicting binary" >&2
    exit 1
fi
echo "Test 3 PASSED."

echo "Test 4: Overwrite existing binary with --force"
CIPPIE_RELEASE_BASE_URL="${LOCAL_BASE_URL}" "${INSTALL_SCRIPT}" \
    --version "${VERSION}" \
    --prefix "${TEST_PREFIX}" \
    --force

"${TEST_PREFIX}/bin/cippie" version | grep -q "${VERSION}"
echo "Test 4 PASSED."

echo "Test 5: Checksum mismatch rejection"
BAD_SERVER_DIR="${TMP_WORK_DIR}/bad_server"
mkdir -p "${BAD_SERVER_DIR}"
cp "$TARBALL" "${BAD_SERVER_DIR}/"
echo "0000000000000000000000000000000000000000000000000000000000000000  cippie-${VERSION}-linux-x86_64.tar.gz" > "${BAD_SERVER_DIR}/cippie-${VERSION}-linux-x86_64.tar.gz.sha256"

if CIPPIE_RELEASE_BASE_URL="file://${BAD_SERVER_DIR}" "${INSTALL_SCRIPT}" \
    --version "${VERSION}" \
    --install-dir "${TMP_WORK_DIR}/bad_dest" 2>/dev/null; then
    echo "ERROR: Install with bad checksum should have failed" >&2
    exit 1
fi
test ! -f "${TMP_WORK_DIR}/bad_dest/cippie"
echo "Test 5 PASSED."

echo "Test 6: Uninstall script"
"${UNINSTALL_SCRIPT}" --prefix "${TEST_PREFIX}"
test ! -f "${TEST_PREFIX}/bin/cippie"
echo "Test 6 PASSED."

echo "Test 7: Uninstall non-existent binary (idempotent)"
"${UNINSTALL_SCRIPT}" --prefix "${TEST_PREFIX}"
echo "Test 7 PASSED."

echo "=== All Installer Integration Tests Passed! ==="
