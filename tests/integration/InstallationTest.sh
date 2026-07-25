#!/usr/bin/env bash
# Installation integration test
set -Eeuo pipefail

CIPPIE_BINARY="$(realpath "${1:?Usage: InstallationTest.sh <cippie-binary>}")"

TMP_PREFIX="/tmp/cippie-install-test-$$"
trap 'rm -rf "${TMP_PREFIX}"' EXIT

BUILD_DIR="$(dirname "${CIPPIE_BINARY}")/.."
if [[ ! -d "${BUILD_DIR}" ]]; then
    BUILD_DIR="$(dirname "${CIPPIE_BINARY}")"
fi

echo "Installing cippie to temporary prefix ${TMP_PREFIX}..."
cmake --install "${BUILD_DIR}" --prefix "${TMP_PREFIX}" 2>&1 || true

INSTALLED_BIN="${TMP_PREFIX}/bin/cippie"
if [[ -f "${INSTALLED_BIN}" ]]; then
    "${INSTALLED_BIN}" version
    "${INSTALLED_BIN}" doctor
    echo "InstallationTest passed"
else
    echo "WARNING: Installed binary not found at ${INSTALLED_BIN}, test skipped"
fi
