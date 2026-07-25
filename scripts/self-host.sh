#!/usr/bin/env bash
# Cippie Self-Host Verification Script
set -Eeuo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

BUILD_FLAG="--debug"
BOOTSTRAP_PATH=""

show_help() {
    cat <<'EOF'
Cippie Self-Host Verification

Usage:
  self-host.sh [options]

Options:
  --release          Build release configuration
  --bootstrap <path> Path to bootstrap cippie executable
  -h, --help         Show this help message
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --release)
            BUILD_FLAG="--release"
            shift
            ;;
        --bootstrap)
            BOOTSTRAP_PATH="${2:?missing bootstrap path}"
            shift 2
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            echo "error: unrecognized option '$1'" >&2
            exit 2
            ;;
    esac
done

# Locate bootstrap binary
if [[ -z "$BOOTSTRAP_PATH" ]]; then
    if [[ -f "${ROOT_DIR}/build/cippie" ]]; then
        BOOTSTRAP_PATH="${ROOT_DIR}/build/cippie"
    elif command -v cippie >/dev/null 2>&1; then
        BOOTSTRAP_PATH="$(command -v cippie)"
    else
        echo "error: no bootstrap Cippie binary found. Build CMake target first or pass --bootstrap." >&2
        exit 1
    fi
fi

BOOTSTRAP_CIPPIE="$(realpath "$BOOTSTRAP_PATH")"
if [[ ! -x "$BOOTSTRAP_CIPPIE" ]]; then
    echo "error: bootstrap Cippie binary is not executable at ${BOOTSTRAP_CIPPIE}" >&2
    exit 1
fi

echo "=== Cippie 2-Generation Self-Host Verification ==="
echo "Bootstrap Cippie: ${BOOTSTRAP_CIPPIE}"
echo "Repository Root : ${ROOT_DIR}"

SELF_HOST_DIR="${ROOT_DIR}/.cippie/self-host"
GEN1_DIR="${SELF_HOST_DIR}/gen1"
GEN2_DIR="${SELF_HOST_DIR}/gen2"

# Clean previous self-host temporary directories
rm -rf "$SELF_HOST_DIR"
mkdir -p "$GEN1_DIR" "$GEN2_DIR"

# 1. Build Gen1 using Bootstrap Cippie
echo "Step 1: Building Gen1 Cippie using bootstrap binary..."
(
    cd "${ROOT_DIR}"
    "${BOOTSTRAP_CIPPIE}" build -j4 "${BUILD_FLAG}" cippie
)

GEN1_BUILT="$(find "${ROOT_DIR}/.cippie/build" -name 'cippie' -type f 2>/dev/null | head -1)"
if [[ -z "${GEN1_BUILT}" || ! -f "${GEN1_BUILT}" ]]; then
    echo "error: Gen1 build artifact not found" >&2
    exit 1
fi

GEN1_BIN="${GEN1_DIR}/cippie"
cp "${GEN1_BUILT}" "${GEN1_BIN}"
chmod +x "${GEN1_BIN}"

echo "Gen1 binary created at ${GEN1_BIN}"

# Verify Gen1
echo "Verifying Gen1 execution..."
GEN1_VER="$("${GEN1_BIN}" version)"
echo "Gen1 Version: ${GEN1_VER}"
"${GEN1_BIN}" help | grep -q "build"
"${GEN1_BIN}" doctor >/dev/null

# 2. Build Gen2 using Gen1 Cippie
echo "Step 2: Building Gen2 Cippie using Gen1 binary..."
(
    cd "${ROOT_DIR}"
    # Remove previous build manifest to force clean Gen2 build
    rm -rf "${ROOT_DIR}/.cippie/build"
    "${GEN1_BIN}" build -j4 "${BUILD_FLAG}" cippie
)

GEN2_BUILT="$(find "${ROOT_DIR}/.cippie/build" -name 'cippie' -type f 2>/dev/null | head -1)"
if [[ -z "${GEN2_BUILT}" || ! -f "${GEN2_BUILT}" ]]; then
    echo "error: Gen2 build artifact not found" >&2
    exit 1
fi

GEN2_BIN="${GEN2_DIR}/cippie"
cp "${GEN2_BUILT}" "${GEN2_BIN}"
chmod +x "${GEN2_BIN}"

echo "Gen2 binary created at ${GEN2_BIN}"

# Verify Gen2
echo "Verifying Gen2 execution..."
GEN2_VER="$("${GEN2_BIN}" version)"
echo "Gen2 Version: ${GEN2_VER}"
"${GEN2_BIN}" help | grep -q "build"
"${GEN2_BIN}" doctor >/dev/null

# 3. Compare Gen1 and Gen2 Behavior
echo "Step 3: Comparing Gen1 and Gen2 output..."
if [[ "${GEN1_VER}" != "${GEN2_VER}" ]]; then
    echo "error: Gen1 version ('${GEN1_VER}') does not match Gen2 version ('${GEN2_VER}')" >&2
    exit 1
fi

# Smoke fixture test with both Gen1 and Gen2
SMOKE_FIXTURE="${ROOT_DIR}/tests/fixtures/project-a-basic"
if [[ -d "${SMOKE_FIXTURE}" ]]; then
    echo "Testing project-a-basic fixture with Gen1..."
    TMP_SMOKE_1="$(mktemp -d)"
    cp -r "${SMOKE_FIXTURE}/." "${TMP_SMOKE_1}/"
    (
        cd "${TMP_SMOKE_1}"
        "${GEN1_BIN}" build
        "${GEN1_BIN}" run | grep -q "Hello"
    )
    rm -rf "${TMP_SMOKE_1}"

    echo "Testing project-a-basic fixture with Gen2..."
    TMP_SMOKE_2="$(mktemp -d)"
    cp -r "${SMOKE_FIXTURE}/." "${TMP_SMOKE_2}/"
    (
        cd "${TMP_SMOKE_2}"
        "${GEN2_BIN}" build
        "${GEN2_BIN}" run | grep -q "Hello"
    )
    rm -rf "${TMP_SMOKE_2}"
fi

echo "=== Cippie Self-Host Verification Succeeded! ==="
echo "Gen1: ${GEN1_BIN} (${GEN1_VER})"
echo "Gen2: ${GEN2_BIN} (${GEN2_VER})"
