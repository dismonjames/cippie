#!/usr/bin/env bash
# Self-hosting first-generation test
# Builds cippie using cippie and verifies the resulting binary works
set -Eeuo pipefail

CIPPIE_BINARY="$(realpath "${1:?Usage: SelfHostFirstGenTest.sh <cippie-binary>}")"
CIPPIE_SRC_DIR="${2:-/home/minh/Documents/cippie}"

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "${TMP_DIR}"' EXIT

echo "=== Self-hosting first generation test ==="
echo "Using cippie binary: ${CIPPIE_BINARY}"
echo "Using cippie source: ${CIPPIE_SRC_DIR}"

# Copy cippie source to temp dir for isolation
cp -r "${CIPPIE_SRC_DIR}/." "${TMP_DIR}/"

# Remove any existing build artifacts from the copy
rm -rf "${TMP_DIR}/.cippie" "${TMP_DIR}/build" "${TMP_DIR}/build-asan"

# Run cippie build in the temp dir
pushd "${TMP_DIR}" >/dev/null

echo "Building cippie from Cippiefile..."
"${CIPPIE_BINARY}" build -j4 cippie

# Find the generated binary
HOST_TRIPLE=$("${CIPPIE_BINARY}" doctor 2>/dev/null | grep "^Host triple" | awk '{print $NF}' || echo "")

GEN1=""
if [[ -n "${HOST_TRIPLE}" ]]; then
    GEN1="${TMP_DIR}/.cippie/build/${HOST_TRIPLE}/debug/cippie/bin/cippie"
fi

if [[ -z "${GEN1}" || ! -f "${GEN1}" ]]; then
    # Try to find it
    GEN1=$(find "${TMP_DIR}/.cippie" -name 'cippie' -type f 2>/dev/null | head -1)
fi

if [[ -z "${GEN1}" || ! -f "${GEN1}" ]]; then
    echo "ERROR: Could not find built cippie binary under ${TMP_DIR}/.cippie" >&2
    exit 1
fi

echo "Found gen1 binary: ${GEN1}"

# Verify gen1 works
"${GEN1}" version
"${GEN1}" help | grep -q "build"
echo "gen1 binary verified successfully"

popd >/dev/null
echo "SelfHostFirstGenTest passed"
