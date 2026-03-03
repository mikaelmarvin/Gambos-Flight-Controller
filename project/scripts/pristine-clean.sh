#!/bin/bash
# Remove the entire build directory. Run from repo root or project/scripts/.
# You must run cmake --preset custom from project/ again before building.
set -e
cd "$(dirname "$0")/.."
rm -rf build
echo "Removed build/. Run 'cd project && cmake --preset custom' before building again."
