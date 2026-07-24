#!/usr/bin/env bash
set -Eeuo pipefail

CIPPIE_BINARY="${1:?missing cippie binary path}"

"${CIPPIE_BINARY}" version
"${CIPPIE_BINARY}" help
