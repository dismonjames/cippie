#!/usr/bin/env bash
# Doctor command integration test
set -Eeuo pipefail

CIPPIE_BINARY="${1:?Usage: DoctorTest.sh <cippie-binary>}"

# doctor command should succeed and print expected fields
output=$("${CIPPIE_BINARY}" doctor)

echo "${output}"

if ! echo "${output}" | grep -q "Host triple"; then
    echo "ERROR: Missing 'Host triple' in doctor output" >&2
    exit 1
fi

if ! echo "${output}" | grep -q "Target triple"; then
    echo "ERROR: Missing 'Target triple' in doctor output" >&2
    exit 1
fi

if ! echo "${output}" | grep -q "Compiler (CXX)"; then
    echo "ERROR: Missing 'Compiler (CXX)' in doctor output" >&2
    exit 1
fi

if ! echo "${output}" | grep -q "Toolchain name"; then
    echo "ERROR: Missing 'Toolchain name' in doctor output" >&2
    exit 1
fi

if ! echo "${output}" | grep -q "Parallel jobs"; then
    echo "ERROR: Missing 'Parallel jobs' in doctor output" >&2
    exit 1
fi

echo "DoctorTest passed"
