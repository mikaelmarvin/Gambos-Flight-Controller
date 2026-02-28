#!/bin/bash
# Clean the Debug build. Run from repo root.
set -e
cd "$(dirname "$0")/.."
cmake --build build/Debug --target clean
