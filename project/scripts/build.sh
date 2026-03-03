#!/bin/bash
# Build the custom target. Run from repo root.
set -e
cd "$(dirname "$0")/.."
cmake --build build/custom -- -j
