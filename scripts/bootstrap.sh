#!/usr/bin/env bash
# Cippie Bootstrap & Source Installer Script
set -Eeuo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

PREFIX=""
BUILD_TYPE="Release"
JOBS="4"
VERIFY_SELF_HOST=false
RUN_TESTS=true

show_help() {
    cat <<'EOF'
Cippie Bootstrap Script

Usage:
  bootstrap.sh [options]

Options:
  --prefix <dir>       Install directory prefix (e.g. $HOME/.local)
  --debug              Build debug configuration
  --release            Build release configuration (default)
  --jobs <count>       Parallel build jobs (default: 4)
  --verify-self-host   Perform full 2-generation self-host verification
  --no-tests           Skip test execution (UNSAFE FOR RELEASE USE)
  -h, --help           Show this help message
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --prefix)
            PREFIX="${2:?missing prefix directory}"
            shift 2
            ;;
        --prefix=*)
            PREFIX="${1#*=}"
            shift
            ;;
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --release)
            BUILD_TYPE="Release"
            shift
            ;;
        --jobs)
            JOBS="${2:?missing jobs count}"
            shift 2
            ;;
        --jobs=*)
            JOBS="${1#*=}"
            shift
            ;;
        --verify-self-host)
            VERIFY_SELF_HOST=true
            shift
            ;;
        --no-tests)
            RUN_TESTS=false
            echo "WARNING: --no-tests flag supplied. Tests skipped (UNSAFE FOR RELEASE USE)." >&2
            shift
            ;;
        --help|-h)
            show_help
            exit 0
            ;;
        *)
            echo "error: unrecognized option '$1'" >&2
            exit 2
            ;;
    esac
done

echo "=== Cippie Bootstrap (${BUILD_TYPE}) ==="
echo "Root directory: ${ROOT_DIR}"

# 1. Detect required tools
command -v cmake >/dev/null 2>&1 || { echo "ERROR: cmake is required but not installed." >&2; exit 1; }

BUILD_GENERATOR="Ninja"
if ! command -v ninja >/dev/null 2>&1; then
    BUILD_GENERATOR="Unix Makefiles"
fi

echo "Using generator: ${BUILD_GENERATOR}"

# 2. Build temporary CMake bootstrap compiler
echo "Step 1: Building CMake bootstrap binary..."
cmake -S "${ROOT_DIR}" -B "${ROOT_DIR}/build" -G "${BUILD_GENERATOR}" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

cmake --build "${ROOT_DIR}/build"

BOOTSTRAP_CIPPIE="${ROOT_DIR}/build/cippie"
if [[ ! -f "${BOOTSTRAP_CIPPIE}" ]]; then
    echo "ERROR: CMake bootstrap binary missing at ${BOOTSTRAP_CIPPIE}" >&2
    exit 1
fi

# 3. Use bootstrap Cippie to build Cippie with Cippie (Self-Host Gen1)
echo "Step 2: Building Cippie with Cippie (Self-Host Gen1)..."
BUILD_FLAG="--release"
if [[ "${BUILD_TYPE}" == "Debug" ]]; then
    BUILD_FLAG="--debug"
fi

(
    cd "${ROOT_DIR}"
    "${BOOTSTRAP_CIPPIE}" build -j"${JOBS}" "${BUILD_FLAG}" cippie
)

# Locate Gen1 binary
HOST_TRIPLE=$("${BOOTSTRAP_CIPPIE}" doctor 2>/dev/null | grep "^Host triple" | awk '{print $NF}' || echo "")
CONFIG_DIR="release"
if [[ "${BUILD_TYPE}" == "Debug" ]]; then
    CONFIG_DIR="debug"
fi

GEN1_CIPPIE=""
if [[ -n "${HOST_TRIPLE}" ]]; then
    GEN1_CIPPIE="${ROOT_DIR}/.cippie/build/${HOST_TRIPLE}/${CONFIG_DIR}/cippie/bin/cippie"
fi

if [[ -z "${GEN1_CIPPIE}" || ! -f "${GEN1_CIPPIE}" ]]; then
    GEN1_CIPPIE=$(find "${ROOT_DIR}/.cippie/build" -name 'cippie' -type f 2>/dev/null | head -1)
fi

if [[ -z "${GEN1_CIPPIE}" || ! -f "${GEN1_CIPPIE}" ]]; then
    echo "ERROR: Could not locate Gen1 self-hosted Cippie binary" >&2
    exit 1
fi

echo "Self-hosted Gen1 binary: ${GEN1_CIPPIE}"

# 4. Optional Gen2 Verification
if [[ "${VERIFY_SELF_HOST}" == "true" ]]; then
    echo "Step 3: Verifying 2-generation self-host capability..."
    if [[ -x "${SCRIPT_DIR}/self-host.sh" ]]; then
        "${SCRIPT_DIR}/self-host.sh" --bootstrap "${BOOTSTRAP_CIPPIE}"
    else
        echo "WARNING: self-host.sh script not found, skipping Gen2 step" >&2
    fi
fi

# 5. Run test suite using self-hosted binary
if [[ "${RUN_TESTS}" == "true" ]]; then
    echo "Step 4: Running CTest test suite..."
    ctest --test-dir "${ROOT_DIR}/build" --output-on-failure
else
    echo "Skipping test suite (--no-tests active)."
fi

# 6. Install self-hosted binary if prefix requested
if [[ -n "${PREFIX}" ]]; then
    DEST_DIR="${PREFIX}/bin"
    DEST_BIN="${DEST_DIR}/cippie"
    echo "Step 5: Installing self-hosted Cippie binary to ${DEST_BIN}..."
    mkdir -p "${DEST_DIR}"

    TMP_DEST="${DEST_DIR}/.cippie.tmp.$$"
    cp "${GEN1_CIPPIE}" "${TMP_DEST}"
    chmod +x "${TMP_DEST}"

    INST_VER="$("${TMP_DEST}" version 2>/dev/null | awk '{print $2}' || echo "")"
    if [[ -z "${INST_VER}" ]]; then
        rm -f "${TMP_DEST}"
        echo "ERROR: Installed binary verification failed" >&2
        exit 1
    fi

    mv -f "${TMP_DEST}" "${DEST_BIN}"
    echo "Cippie ${INST_VER} installed successfully to ${DEST_BIN}"
    "${DEST_BIN}" version
else
    echo "Cippie bootstrap build complete: ${GEN1_CIPPIE}"
fi
