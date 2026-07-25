#!/usr/bin/env bash
# Self-hosting second-generation test
# Builds cippie using gen1 cippie, then verifies gen2 also works
set -Eeuo pipefail

CIPPIE_BINARY="$(realpath "${1:?Usage: SelfHostSecondGenTest.sh <cippie-binary>}")"
CIPPIE_SRC_DIR="${2:-/home/minh/Documents/cippie}"

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "${TMP_DIR}"' EXIT

echo "=== Self-hosting second generation test ==="

# Copy cippie source
cp -r "${CIPPIE_SRC_DIR}/." "${TMP_DIR}/"
rm -rf "${TMP_DIR}/.cippie" "${TMP_DIR}/build" "${TMP_DIR}/build-asan"

pushd "${TMP_DIR}" >/dev/null

# Gen1: build with bootstrap cippie
echo "Building gen1..."
"${CIPPIE_BINARY}" build -j4 cippie

GEN1=$(find "${TMP_DIR}/.cippie" -name 'cippie' -type f 2>/dev/null | head -1)
if [[ -z "${GEN1}" || ! -f "${GEN1}" ]]; then
    echo "ERROR: gen1 binary not found" >&2
    exit 1
fi
echo "gen1: ${GEN1}"
GEN1_SAFE="${TMP_DIR}/cippie_gen1"
cp "${GEN1}" "${GEN1_SAFE}"
echo "Building gen2..."
rm -rf "${TMP_DIR}/.cippie"
"${GEN1_SAFE}" build -j4 cippie

GEN2=$(find "${TMP_DIR}/.cippie" -name 'cippie' -type f 2>/dev/null | head -1)
if [[ -z "${GEN2}" || ! -f "${GEN2}" ]]; then
    echo "ERROR: gen2 binary not found" >&2
    exit 1
fi
echo "gen2: ${GEN2}"
"${GEN2}" version
"${GEN2}" help | grep -q "build"

echo "gen2 binary verified successfully"
popd >/dev/null
echo "SelfHostSecondGenTest passed"
