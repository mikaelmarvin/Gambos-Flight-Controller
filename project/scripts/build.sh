#!/bin/bash
# Build the Debug target. Run from repo root.
set -e
cd "$(dirname "$0")/.."
cmake --build build/Debug -- -j
