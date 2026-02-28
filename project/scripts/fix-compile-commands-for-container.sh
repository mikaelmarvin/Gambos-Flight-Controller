#!/bin/bash
# Rewrite compile_commands.json so all paths use the container workspace path.
# This makes clangd (and Go to Definition / headers) work when developing
# inside the dev container. Run from repo root; safe to run multiple times.
# If compile_commands.json was built on the host (e.g. /home/user/gambos-sw),
# this script rewrites it to /workspace/project/gambos-sw.

set -e
REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
CC_JSON="${REPO_ROOT}/project/build/Debug/compile_commands.json"
CONTAINER_WORKSPACE="${CONTAINER_WORKSPACE:-/workspace/project/gambos-sw}"

if [[ ! -f "$CC_JSON" ]]; then
  echo "Not found: $CC_JSON. Run project/scripts/configure.sh first."
  exit 0
fi

# Detect host path from first entry (e.g. /home/mikael/gambos-sw/project/build/Debug)
FIRST_DIR=$(grep -oP '"directory":\s*"\K[^"]+' "$CC_JSON" | head -1)
if [[ -z "$FIRST_DIR" ]]; then
  echo "Could not parse directory from compile_commands.json"
  exit 1
fi

# If already using container path, nothing to do
if [[ "$FIRST_DIR" == /workspace/* ]]; then
  echo "compile_commands.json already uses container paths."
  exit 0
fi

# Derive host repo root: path is like /home/user/gambos-sw/project/build/Debug
# Take everything up to and including "gambos-sw"
if [[ "$FIRST_DIR" =~ ^(.*/gambos-sw) ]]; then
  HOST_ROOT="${BASH_REMATCH[1]}"
else
  echo "Could not detect host repo root from: $FIRST_DIR"
  exit 1
fi

if [[ "$HOST_ROOT" == "$CONTAINER_WORKSPACE" ]]; then
  echo "compile_commands.json already uses container paths."
  exit 0
fi

sed -i "s|${HOST_ROOT}|${CONTAINER_WORKSPACE}|g" "$CC_JSON"
echo "Updated compile_commands.json ($HOST_ROOT -> $CONTAINER_WORKSPACE). Reload the window (Ctrl+Shift+P -> Developer: Reload Window) so clangd picks it up."
