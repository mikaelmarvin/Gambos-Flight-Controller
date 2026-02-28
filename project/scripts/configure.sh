#!/bin/bash
# Generate build files and compile_commands.json (for editor Go to Definition).
# Run from repo root or project/scripts/. Needs: cmake, ninja, arm-none-eabi-gcc
# (e.g. in the dev container, or in WSL after: apt install cmake ninja-build gcc-arm-none-eabi)

set -e
cd "$(dirname "$0")/.."
cmake --preset Debug
echo "Done. compile_commands.json is in build/Debug/"
echo "Reload the editor window (Ctrl+Shift+P -> Developer: Reload Window) so clangd picks it up."
