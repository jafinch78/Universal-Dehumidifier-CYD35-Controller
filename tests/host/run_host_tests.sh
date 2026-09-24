#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="${TMPDIR:-/tmp}/universal_dehumidifier_host_tests"
g++ -std=c++17 -Wall -Wextra -Werror "$ROOT/tests/host/test_main.cpp" "$ROOT/firmware/UniversalDehumidifier/compressor_guard.cpp" "$ROOT/firmware/UniversalDehumidifier/controller.cpp" -I"$ROOT/firmware/UniversalDehumidifier" -o "$OUT"
"$OUT"
