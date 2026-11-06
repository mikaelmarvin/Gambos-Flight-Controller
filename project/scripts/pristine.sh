#!/usr/bin/env bash
# Deletes the entire build folder

set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

rm -rf build
echo "OK: removed project/build/."
echo "Run: ./project/scripts/build.sh"
exit 0

