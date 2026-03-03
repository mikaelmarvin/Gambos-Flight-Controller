#!/bin/bash
# Clean the custom build. Run from repo root.
set -e
cd "$(dirname "$0")/.."
cmake --build build/custom --target clean
