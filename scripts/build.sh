#!/usr/bin/env bash
# Cippie Primary Build Wrapper Script
set -Eeuo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

ALLOW_BOOTSTRAP=false
PASSTHROUGH_ARGS=()

while [[ $# -gt 0 ]]; do
    case "$1" in
        --bootstrap)
            ALLOW_BOOTSTRAP=true
            shift
            ;;
        *)
            PASSTHROUGH_ARGS+=("$1")
            shift
            ;;
    esac
done

CIPPIE_EXEC=""

if command -v cippie >/dev/null 2>&1; then
    CIPPIE_EXEC="cippie"
elif [[ -x "${ROOT_DIR}/build/cippie" ]]; then
    CIPPIE_EXEC="${ROOT_DIR}/build/cippie"
fi

if [[ -z "$CIPPIE_EXEC" ]]; then
    if [[ "$ALLOW_BOOTSTRAP" == "true" ]]; then
        echo "Cippie is not installed. Bootstrapping via CMake..."
        "${SCRIPT_DIR}/bootstrap.sh"
        CIPPIE_EXEC="${ROOT_DIR}/build/cippie"
    else
        echo "error: Cippie binary not found in PATH or ${ROOT_DIR}/build/cippie." >&2
        echo "Run './scripts/bootstrap.sh' or pass '--bootstrap' to build from scratch." >&2
        exit 1
    fi
fi

cd "${ROOT_DIR}"
exec "${CIPPIE_EXEC}" build "${PASSTHROUGH_ARGS[@]+"${PASSTHROUGH_ARGS[@]}"}"
