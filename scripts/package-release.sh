#!/usr/bin/env bash
# Release packaging script for Cippie
set -Eeuo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

VERSION="0.1.0"
HOST_ARCH="$(uname -m)"
HOST_OS="$(uname -s | tr '[:upper:]' '[:lower:]')"
PACKAGE_NAME="cippie-${VERSION}-${HOST_OS}-${HOST_ARCH}"
DIST_DIR="${ROOT_DIR}/dist"
STAGE_DIR="$(mktemp -d)"

trap 'rm -rf "${STAGE_DIR}"' EXIT

echo "=== Packaging Cippie Release ${VERSION} (${PACKAGE_NAME}) ==="

# 1. Build and test Release binary
echo "Building Release binary..."
cmake -S "${ROOT_DIR}" -B "${ROOT_DIR}/build" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

cmake --build "${ROOT_DIR}/build"

echo "Running test suite before packaging..."
ctest --test-dir "${ROOT_DIR}/build" --output-on-failure

# 2. Prepare staging layout
TARGET_DIR="${STAGE_DIR}/${PACKAGE_NAME}"
mkdir -p "${TARGET_DIR}/bin"

cp "${ROOT_DIR}/build/cippie" "${TARGET_DIR}/bin/cippie"
cp "${ROOT_DIR}/LICENSE" "${TARGET_DIR}/LICENSE"
cp "${ROOT_DIR}/README.md" "${TARGET_DIR}/README.md"
cp "${ROOT_DIR}/CHANGELOG.md" "${TARGET_DIR}/CHANGELOG.md"

# 3. Create release tarball
mkdir -p "${DIST_DIR}"
TARBALL="${DIST_DIR}/${PACKAGE_NAME}.tar.gz"
CHECKSUM="${DIST_DIR}/${PACKAGE_NAME}.tar.gz.sha256"

echo "Creating release archive ${TARBALL}..."
tar -czf "${TARBALL}" -C "${STAGE_DIR}" "${PACKAGE_NAME}"

# 4. Generate SHA-256 checksum
(cd "${DIST_DIR}" && sha256sum "${PACKAGE_NAME}.tar.gz" > "${PACKAGE_NAME}.tar.gz.sha256")

echo "Release package created successfully:"
echo "  Archive : ${TARBALL}"
echo "  Checksum: ${CHECKSUM}"
cat "${CHECKSUM}"
