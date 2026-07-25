#!/usr/bin/env bash
# Cippie Bootstrap Script
set -Eeuo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

PREFIX=""
BUILD_TYPE="Release"

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
        --help|-h)
            echo "Usage: $0 [--prefix <dir>] [--debug]"
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            exit 2
            ;;
    esac
done

echo "=== Cippie Bootstrap (${BUILD_TYPE}) ==="
echo "Root directory: ${ROOT_DIR}"

# Check required commands
command -v cmake >/dev/null 2>&1 || { echo "ERROR: cmake is required but not installed." >&2; exit 1; }

BUILD_GENERATOR="Ninja"
if ! command -v ninja >/dev/null 2>&1; then
    BUILD_GENERATOR="Unix Makefiles"
fi

echo "Using generator: ${BUILD_GENERATOR}"

# 1. Configure
cmake -S "${ROOT_DIR}" -B "${ROOT_DIR}/build" -G "${BUILD_GENERATOR}" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# 2. Build
cmake --build "${ROOT_DIR}/build"

# 3. Test
ctest --test-dir "${ROOT_DIR}/build" --output-on-failure

# 4. Install if prefix requested
if [[ -n "${PREFIX}" ]]; then
    echo "Installing Cippie to ${PREFIX}..."
    cmake --install "${ROOT_DIR}/build" --prefix "${PREFIX}"
    echo "Cippie installed successfully to ${PREFIX}/bin/cippie"
else
    echo "Cippie bootstrap build complete: ${ROOT_DIR}/build/cippie"
fi
