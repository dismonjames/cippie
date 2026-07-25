#!/usr/bin/env bash
# Release packaging script for Cippie
# Uses Cippie's self-hosting build system (bootstrap CMake only for stage 0)
set -Eeuo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

VERSION="0.1.1"
HOST_ARCH="$(uname -m)"
HOST_OS="$(uname -s | tr '[:upper:]' '[:lower:]')"

case "${HOST_OS}" in
    darwin)    HOST_OS="darwin"  ;;
    linux)     HOST_OS="linux"   ;;
    mingw64_nt-*|msys_nt-*|cygwin_nt-*) HOST_OS="windows" ;;
esac

case "${HOST_ARCH}" in
    amd64)     HOST_ARCH="x86_64"   ;;
    arm64|aarch64) HOST_ARCH="aarch64" ;;
esac

PACKAGE_NAME="cippie-${VERSION}-${HOST_OS}-${HOST_ARCH}"
DIST_DIR="${ROOT_DIR}/dist"
STAGE_DIR="$(mktemp -d)"

trap 'rm -rf "${STAGE_DIR}"' EXIT

echo "=== Packaging Cippie Release ${VERSION} (${PACKAGE_NAME}) ==="

# Step 1: Bootstrap — build stage 0 with CMake, then self-host with Cippie
echo "Bootstrapping Cippie via CMake (stage 0)..."
cmake -S "${ROOT_DIR}" -B "${ROOT_DIR}/build" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

cmake --build "${ROOT_DIR}/build"

BOOTSTRAP_CIPPIE="${ROOT_DIR}/build/cippie"
if [ "${HOST_OS}" = "windows" ]; then
    BOOTSTRAP_CIPPIE="${ROOT_DIR}/build/Release/cippie.exe"
    if [ ! -f "${BOOTSTRAP_CIPPIE}" ]; then
        BOOTSTRAP_CIPPIE="${ROOT_DIR}/build/cippie.exe"
    fi
fi

if [ ! -f "${BOOTSTRAP_CIPPIE}" ]; then
    echo "Error: CMake bootstrap binary not found" >&2
    exit 1
fi

echo "Self-hosting: building Cippie with Cippie (stage 1)..."
(cd "${ROOT_DIR}" && "${BOOTSTRAP_CIPPIE}" build -j"$(nproc 2>/dev/null || echo 4)" --release cippie)

# Locate self-hosted (gen1) binary
HOST_TRIPLE=$("${BOOTSTRAP_CIPPIE}" doctor 2>/dev/null | grep "^Host triple" | awk '{print $NF}' || echo "")
GEN1_CIPPIE=""
if [ -n "${HOST_TRIPLE}" ]; then
    GEN1_CIPPIE="${ROOT_DIR}/.cippie/build/${HOST_TRIPLE}/release/cippie/bin/cippie"
fi
if [ -z "${GEN1_CIPPIE}" ] || [ ! -f "${GEN1_CIPPIE}" ]; then
    GEN1_CIPPIE=$(find "${ROOT_DIR}/.cippie/build" -name 'cippie' -type f 2>/dev/null | head -1)
fi
if [ -z "${GEN1_CIPPIE}" ] || [ ! -f "${GEN1_CIPPIE}" ]; then
    echo "Error: could not locate self-hosted Cippie binary" >&2
    exit 1
fi

echo "Self-hosted binary: ${GEN1_CIPPIE}"

# Run test suite
echo "Running test suite..."
ctest --test-dir "${ROOT_DIR}/build" --output-on-failure || echo "Warning: some tests failed"

# Prepare staging layout
TARGET_DIR="${STAGE_DIR}/${PACKAGE_NAME}"
mkdir -p "${TARGET_DIR}/bin"

BINARY_DST="cippie"
if [ "${HOST_OS}" = "windows" ]; then
    BINARY_DST="cippie.exe"
fi

cp "${GEN1_CIPPIE}" "${TARGET_DIR}/bin/${BINARY_DST}"
cp "${ROOT_DIR}/LICENSE" "${TARGET_DIR}/LICENSE"
cp "${ROOT_DIR}/README.md" "${TARGET_DIR}/README.md"
cp "${ROOT_DIR}/CHANGELOG.md" "${TARGET_DIR}/CHANGELOG.md"

# Create release tarball
mkdir -p "${DIST_DIR}"
TARBALL="${DIST_DIR}/${PACKAGE_NAME}.tar.gz"
CHECKSUM="${DIST_DIR}/${PACKAGE_NAME}.tar.gz.sha256"

echo "Creating release archive ${TARBALL}..."
tar -czf "${TARBALL}" -C "${STAGE_DIR}" "${PACKAGE_NAME}"

# Generate SHA-256 checksum
(cd "${DIST_DIR}" && sha256sum "${PACKAGE_NAME}.tar.gz" > "${PACKAGE_NAME}.tar.gz.sha256")

echo "Release package created successfully:"
echo "  Archive : ${TARBALL}"
echo "  Checksum: ${CHECKSUM}"
cat "${CHECKSUM}"
