#!/usr/bin/env bash
# Release packaging script for Cippie
# Uses previous Cippie release to self-host build the new version
set -Eeuo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

VERSION="0.1.3"
BOOTSTRAP_WITH_CMAKE=false
CIPPIE_BIN=""

show_help() {
    cat <<'EOF'
Usage: package-release.sh [options]

Options:
  --version <ver>      Version string for the package (default: 0.1.2)
  --cippie-bin <path>  Path to an existing Cippie binary to use for building
  --cmake-bootstrap    Fall back to CMake bootstrap if no Cippie binary is available
  --help               Show this help message
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --version)
            VERSION="$2"; shift 2 ;;
        --version=*)
            VERSION="${1#*=}"; shift ;;
        --cippie-bin)
            CIPPIE_BIN="$2"; shift 2 ;;
        --cippie-bin=*)
            CIPPIE_BIN="${1#*=}"; shift ;;
        --cmake-bootstrap)
            BOOTSTRAP_WITH_CMAKE=true; shift ;;
        --help|-h)
            show_help; exit 0 ;;
        *)
            echo "error: unrecognized option '$1'" >&2; exit 2 ;;
    esac
done

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
CLEANUP_STAGE=true

trap 'if [ "$CLEANUP_STAGE" = true ]; then rm -rf "${STAGE_DIR}"; fi' EXIT

echo "=== Packaging Cippie Release ${VERSION} (${PACKAGE_NAME}) ==="

# Obtain a Cippie binary to build with
BUILD_CIPPIE=""

if [ -n "$CIPPIE_BIN" ]; then
    BUILD_CIPPIE="$CIPPIE_BIN"
    echo "Using provided Cippie binary: ${BUILD_CIPPIE}"
elif command -v cippie &>/dev/null; then
    BUILD_CIPPIE="$(command -v cippie)"
    echo "Using Cippie from PATH: ${BUILD_CIPPIE}"
elif [ "$BOOTSTRAP_WITH_CMAKE" = true ]; then
    echo "Bootstrapping via CMake..."
    cmake -S "${ROOT_DIR}" -B "${ROOT_DIR}/build" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    cmake --build "${ROOT_DIR}/build"
    BUILD_CIPPIE="${ROOT_DIR}/build/cippie"
    if [ ! -f "$BUILD_CIPPIE" ]; then
        BUILD_CIPPIE="${ROOT_DIR}/build/cippie.exe"
    fi
else
    echo "No Cippie binary available. Downloading latest release..."
    DOWNLOAD_DIR="${STAGE_DIR}/download"
    mkdir -p "$DOWNLOAD_DIR"

    LATEST_TAG=$(curl -fsSL https://api.github.com/repos/dismonjames/cippie/releases/latest \
        | grep '"tag_name"' | head -1 | sed 's/.*"tag_name": *"\(.*\)",*/\1/')
    if [ -z "$LATEST_TAG" ]; then
        echo "Error: could not fetch latest release tag" >&2
        exit 1
    fi
    echo "Latest release: ${LATEST_TAG}"

    LATEST_VERSION="${LATEST_TAG#v}"
    TARBALL_NAME="cippie-${LATEST_VERSION}-${HOST_OS}-${HOST_ARCH}.tar.gz"
    TARBALL_URL="https://github.com/dismonjames/cippie/releases/download/${LATEST_TAG}/${TARBALL_NAME}"

    echo "Downloading ${TARBALL_NAME}..."
    curl -fsSL "$TARBALL_URL" -o "${DOWNLOAD_DIR}/${TARBALL_NAME}"

    echo "Extracting..."
    tar -xzf "${DOWNLOAD_DIR}/${TARBALL_NAME}" -C "$DOWNLOAD_DIR"

    BINARY_NAME="cippie"
    [ "$HOST_OS" = "windows" ] && BINARY_NAME="cippie.exe"
    BUILD_CIPPIE="${DOWNLOAD_DIR}/cippie-${LATEST_VERSION}-${HOST_OS}-${HOST_ARCH}/bin/${BINARY_NAME}"

    if [ ! -f "$BUILD_CIPPIE" ]; then
        echo "Error: downloaded release does not contain expected binary" >&2
        exit 1
    fi
    chmod +x "$BUILD_CIPPIE"
    echo "Using downloaded Cippie ${LATEST_TAG}: ${BUILD_CIPPIE}"
fi

if [ ! -f "$BUILD_CIPPIE" ]; then
    echo "Error: no Cippie binary available for building" >&2
    exit 1
fi

echo "Building Cippie ${VERSION} with Cippie (self-host)..."
(cd "${ROOT_DIR}" && "${BUILD_CIPPIE}" build -j"$(nproc 2>/dev/null || echo 4)" --release cippie)

# Locate self-hosted binary
HOST_TRIPLE=$("${BUILD_CIPPIE}" doctor 2>/dev/null | grep "^Host triple" | awk '{print $NF}' || echo "")
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

# Verify version
BUILT_VER=$("${GEN1_CIPPIE}" version 2>/dev/null | awk '{print $2}' || echo "")
echo "Built Cippie version: ${BUILT_VER}"

# Prepare staging layout
TARGET_DIR="${STAGE_DIR}/${PACKAGE_NAME}"
mkdir -p "${TARGET_DIR}/bin"

BINARY_DST="cippie"
[ "${HOST_OS}" = "windows" ] && BINARY_DST="cippie.exe"

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

CLEANUP_STAGE=false
echo "Release package created successfully:"
echo "  Archive : ${TARBALL}"
echo "  Checksum: ${CHECKSUM}"
cat "${CHECKSUM}"
