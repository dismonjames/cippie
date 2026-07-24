#!/usr/bin/env bash
set -Eeuo pipefail

CIPPIE_BINARY="${1:?missing cippie binary path}"

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "${TMP_DIR}"' EXIT

cd "${TMP_DIR}"

cat <<'EOF' > Cippiefile
project("lock-test") {
    cpp = 23;
    executable("app") {
        entry = "src/main.cpp";
    }
}
EOF

mkdir -p src .cippie/locks
cat <<'EOF' > src/main.cpp
int main() { return 0; }
EOF

# Manually acquire flock on .cippie/locks/build.lock in background
(
    exec 9> .cippie/locks/build.lock
    flock 9
    sleep 2
) &
LOCK_PID=$!

sleep 0.2

# Attempting cippie build while lock is held must fail deterministically
if "${CIPPIE_BINARY}" build; then
    echo "Expected build to fail due to lock conflict!"
    kill -9 "${LOCK_PID}" || true
    exit 1
fi

wait "${LOCK_PID}" || true

# After lock is released, build succeeds
"${CIPPIE_BINARY}" build

echo "LockConflictTest passed successfully"
