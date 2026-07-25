#!/bin/sh
# Cippie Official Linux Binary Installer
# POSIX sh compatible — works with bash, dash, ash
set -eu

VERSION="${CIPPIE_INSTALL_VERSION:-0.1.0}"
PREFIX="${CIPPIE_INSTALL_PREFIX:-$HOME/.local}"
INSTALL_DIR=""
FORCE=false
NO_MODIFY_PATH=false
RELEASE_BASE_URL="${CIPPIE_RELEASE_BASE_URL:-https://github.com/dismonjames/cippie/releases/download}"

show_help() {
    cat <<'EOF'
Cippie Installer

Usage:
  install.sh [options]

Options:
  --version <ver>      Version to install (default: 0.1.0)
  --prefix <path>      Installation prefix (default: $HOME/.local)
  --install-dir <path> Direct binary installation directory
  --force              Overwrite existing installation
  --no-modify-path     Accepted for forward compatibility (installer only prints PATH instructions)
  -h, --help           Show this help message

Environment Variables:
  CIPPIE_INSTALL_VERSION   Version to install
  CIPPIE_INSTALL_PREFIX    Installation prefix
  CIPPIE_RELEASE_BASE_URL  Base download URL (for testing/mirrors; default: GitHub releases)
EOF
}

# Parse CLI arguments
while [ $# -gt 0 ]; do
    case "$1" in
        --version)
            VERSION="${2:?Error: --version requires a version value}"
            shift 2
            ;;
        --prefix)
            PREFIX="${2:?Error: --prefix requires a path value}"
            shift 2
            ;;
        --install-dir)
            INSTALL_DIR="${2:?Error: --install-dir requires a path value}"
            shift 2
            ;;
        --force)
            FORCE=true
            shift
            ;;
        --no-modify-path)
            NO_MODIFY_PATH=true
            shift
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

# Detect OS and Architecture
RAW_OS="$(uname -s)"
RAW_ARCH="$(uname -m)"

OS="$(echo "$RAW_OS" | tr '[:upper:]' '[:lower:]')"
case "$RAW_ARCH" in
    x86_64|amd64)
        ARCH="x86_64"
        ;;
    *)
        ARCH="$RAW_ARCH"
        ;;
esac

if [ "$OS" != "linux" ] || [ "$ARCH" != "x86_64" ]; then
    echo "error: no prebuilt Cippie release is available for ${OS}-${ARCH}" >&2
    exit 1
fi

PLATFORM="${OS}-${ARCH}"
PACKAGE_NAME="cippie-${VERSION}-${PLATFORM}"
TARBALL_NAME="${PACKAGE_NAME}.tar.gz"
CHECKSUM_NAME="${PACKAGE_NAME}.tar.gz.sha256"

if [ -n "$INSTALL_DIR" ]; then
    DEST_DIR="$INSTALL_DIR"
else
    DEST_DIR="${PREFIX}/bin"
fi

DEST_BINARY="${DEST_DIR}/cippie"

# Handle existing binary
if [ -e "$DEST_BINARY" ] && [ "$FORCE" != "true" ]; then
    if [ -x "$DEST_BINARY" ]; then
        INST_VER="$("$DEST_BINARY" version 2>/dev/null | awk '{print $2}' || echo "")"
        if [ "$INST_VER" = "$VERSION" ]; then
            echo "Cippie ${VERSION} is already installed at ${DEST_BINARY}."
            exit 0
        fi
    fi
    echo "error: binary already exists at ${DEST_BINARY}. Use --force to overwrite." >&2
    exit 1
fi

TMP_DIR="$(mktemp -d)"
# shellcheck disable=SC2064
trap "rm -rf '${TMP_DIR}'" EXIT

# Build download URLs
case "$RELEASE_BASE_URL" in
    *github.com*)
        TARBALL_URL="${RELEASE_BASE_URL}/v${VERSION}/${TARBALL_NAME}"
        CHECKSUM_URL="${RELEASE_BASE_URL}/v${VERSION}/${CHECKSUM_NAME}"
        ;;
    *)
        TARBALL_URL="${RELEASE_BASE_URL}/${TARBALL_NAME}"
        CHECKSUM_URL="${RELEASE_BASE_URL}/${CHECKSUM_NAME}"
        ;;
esac

echo "Downloading Cippie ${VERSION} (${PLATFORM})..."

# Download files safely
fetch_url() {
    url="$1"
    output="$2"
    if command -v curl >/dev/null 2>&1; then
        curl -fsSL "$url" -o "$output"
    elif command -v wget >/dev/null 2>&1; then
        wget -q -O "$output" "$url"
    else
        echo "error: curl or wget is required to download Cippie" >&2
        exit 1
    fi
}

if ! fetch_url "$CHECKSUM_URL" "${TMP_DIR}/${CHECKSUM_NAME}"; then
    echo "error: failed to download checksum file" >&2
    exit 1
fi

if ! fetch_url "$TARBALL_URL" "${TMP_DIR}/${TARBALL_NAME}"; then
    echo "error: failed to download release package" >&2
    exit 1
fi

# Verify SHA-256 BEFORE extraction
echo "Verifying SHA-256 checksum..."
(
    cd "$TMP_DIR"
    if command -v sha256sum >/dev/null 2>&1; then
        sha256sum -c "${CHECKSUM_NAME}" >/dev/null 2>&1
    elif command -v shasum >/dev/null 2>&1; then
        shasum -a 256 -c "${CHECKSUM_NAME}" >/dev/null 2>&1
    else
        echo "error: sha256sum or shasum is required" >&2
        exit 1
    fi
) || {
    echo "error: SHA-256 checksum verification failed" >&2
    exit 1
}

# Validate archive layout — reject path traversal
echo "Validating release archive layout..."
TAR_LIST="$(tar -tzf "${TMP_DIR}/${TARBALL_NAME}")"

if echo "$TAR_LIST" | grep -q '\.\.'; then
    echo "error: malicious path traversal detected in archive" >&2
    exit 1
fi

EXPECTED_BIN="${PACKAGE_NAME}/bin/cippie"
if ! echo "$TAR_LIST" | grep -qx "${EXPECTED_BIN}"; then
    echo "error: archive does not contain expected executable ${EXPECTED_BIN}" >&2
    exit 1
fi

# Extract
EXTRACT_DIR="${TMP_DIR}/ext"
mkdir -p "$EXTRACT_DIR"
tar -xzf "${TMP_DIR}/${TARBALL_NAME}" -C "$EXTRACT_DIR"

SOURCE_BINARY="${EXTRACT_DIR}/${EXPECTED_BIN}"
if [ ! -f "$SOURCE_BINARY" ]; then
    echo "error: extracted binary missing" >&2
    exit 1
fi

chmod +x "$SOURCE_BINARY"

# Verify extracted binary
EXTRACTED_VER="$("$SOURCE_BINARY" version 2>/dev/null | awk '{print $2}' || echo "")"
if [ "$EXTRACTED_VER" != "$VERSION" ]; then
    echo "error: extracted binary version ('$EXTRACTED_VER') does not match expected '$VERSION'" >&2
    exit 1
fi

# Atomic installation
mkdir -p "$DEST_DIR"

if [ -d "$DEST_BINARY" ]; then
    echo "error: destination ${DEST_BINARY} is a directory" >&2
    exit 1
fi

TMP_DEST="${DEST_DIR}/.cippie.tmp.$$"
cp "$SOURCE_BINARY" "$TMP_DEST"
chmod +x "$TMP_DEST"

TEST_VER="$("$TMP_DEST" version 2>/dev/null | awk '{print $2}' || echo "")"
if [ "$TEST_VER" != "$VERSION" ]; then
    rm -f "$TMP_DEST"
    echo "error: destination binary verification failed" >&2
    exit 1
fi

mv -f "$TMP_DEST" "$DEST_BINARY"

echo ""
echo "Cippie ${VERSION} installed successfully."
echo ""
echo "Binary:"
echo "  ${DEST_BINARY}"
echo ""

if [ "$NO_MODIFY_PATH" = "true" ]; then
    echo "Note: --no-modify-path active. Shell configuration files were not modified."
fi

# PATH check
case ":${PATH}:" in
    *":${DEST_DIR}:"*)
        ;;
    *)
        echo "Add this directory to your PATH:"
        echo ""
        echo "  export PATH=\"${DEST_DIR}:\$PATH\""
        echo ""
        ;;
esac
