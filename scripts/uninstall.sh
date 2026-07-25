#!/usr/bin/env bash
# Cippie Linux Binary Uninstaller
set -Eeuo pipefail

PREFIX="${CIPPIE_INSTALL_PREFIX:-$HOME/.local}"
INSTALL_DIR=""

show_help() {
    cat <<'EOF'
Cippie Uninstaller

Usage:
  uninstall.sh [options]

Options:
  --prefix <path>      Installation prefix (default: $HOME/.local)
  --install-dir <path> Direct binary installation directory
  -h, --help           Show this help message
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --prefix)
            PREFIX="${2:?Error: --prefix requires a path value}"
            shift 2
            ;;
        --install-dir)
            INSTALL_DIR="${2:?Error: --install-dir requires a path value}"
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

if [[ -n "$INSTALL_DIR" ]]; then
    DEST_DIR="$INSTALL_DIR"
else
    DEST_DIR="${PREFIX}/bin"
fi

TARGET_BINARY="${DEST_DIR}/cippie"
CANONICAL_TARGET="$(realpath -m "$TARGET_BINARY")"

# Safety checks against dangerous paths
case "$CANONICAL_TARGET" in
    /|/bin|/bin/cippie|/usr|/usr/bin|/usr/bin/cippie|/sbin|/usr/sbin|/lib|/home)
        # Note: system paths like /usr/bin/cippie are only allowed if explicitly intended, but root paths are rejected
        if [[ "$CANONICAL_TARGET" == "/" || "$CANONICAL_TARGET" == "/bin" || "$CANONICAL_TARGET" == "/usr" || "$CANONICAL_TARGET" == "/usr/bin" ]]; then
            echo "error: refusing to uninstall from system root path ${CANONICAL_TARGET}" >&2
            exit 1
        fi
        ;;
esac

if [[ ! -f "$TARGET_BINARY" && ! -L "$TARGET_BINARY" ]]; then
    echo "Cippie binary not found at ${TARGET_BINARY}."
    exit 0
fi

echo "Removing Cippie binary: ${TARGET_BINARY}"
rm -f "$TARGET_BINARY"

echo "Cippie has been uninstalled successfully."
echo ""
echo "Note: Configuration and package caches were preserved."
echo "If you wish to remove caches manually, run:"
echo "  rm -rf ~/.cache/cippie ~/.config/cippie"
