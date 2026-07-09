#!/usr/bin/env bash
# Materialize board/gambos-pcb from upstream Cube output + project patches.
#
# Usage:
#   ./software/project/scripts/sync-cubemx-gambos-pcb.sh
#
# Workflow after CubeMX generation:
#   1. Copy/replace board/gambos-pcb_upstream/ with the fresh Cube export
#   2. Run this script
#   3. ./software/project/scripts/build.sh
#
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BOARD="gambos-pcb"
UPSTREAM="${ROOT}/board/${BOARD}_upstream"
PATCHED="${ROOT}/board/${BOARD}"
PATCH_DIR="${ROOT}/board/${BOARD}_patches"
CLANG_FORMAT="${CLANG_FORMAT:-clang-format}"
FORMAT_STYLE="${ROOT}/../.clang-format"

if [[ ! -d "${UPSTREAM}" ]]; then
    echo "Missing upstream board tree: ${UPSTREAM}" >&2
    exit 1
fi

if [[ ! -d "${PATCH_DIR}" ]]; then
    echo "Missing patch directory: ${PATCH_DIR}" >&2
    exit 1
fi

mapfile -t PATCHES < <(find "${PATCH_DIR}" -maxdepth 1 -name '*.patch' -print | sort)
if [[ ${#PATCHES[@]} -eq 0 ]]; then
    echo "No patch files found in ${PATCH_DIR}" >&2
    exit 1
fi

echo "Sync ${BOARD}: ${UPSTREAM} -> ${PATCHED}"
rm -rf "${PATCHED}"
cp -a "${UPSTREAM}" "${PATCHED}"

export PATCHED
python3 - <<'PY'
from pathlib import Path
root = Path(__import__('os').environ['PATCHED'])
exts = {'.c', '.h', '.s', '.txt', '.cmake', '.ioc', '.md', '.ld'}
for path in root.rglob('*'):
    if not path.is_file():
        continue
    if path.suffix not in exts and path.name not in ('CMakeLists.txt', '.mxproject'):
        continue
    data = path.read_bytes()
    if b'\r\n' in data:
        path.write_bytes(data.replace(b'\r\n', b'\n'))
PY

for patch in "${PATCHES[@]}"; do
    echo "Applying $(basename "${patch}")"
    if ! patch -d "${PATCHED}" -p0 --forward --reject-file=- < "${patch}"; then
        echo "Failed to apply patch: ${patch}" >&2
        exit 1
    fi
done

if command -v "${CLANG_FORMAT}" >/dev/null 2>&1 && [[ -f "${FORMAT_STYLE}" ]]; then
  mapfile -t PATCHED_FILES < <(awk '/^\+\+\+ / { sub(/^\+\+\+ /, ""); print }' "${PATCHES[@]}" | sort -u)
  for rel in "${PATCHED_FILES[@]}"; do
    file="${PATCHED}/${rel}"
    if [[ -f "${file}" ]]; then
      "${CLANG_FORMAT}" -i -style=file "${file}"
    fi
  done
fi

cat > "${PATCHED}/README.md" <<'EOF'
# Patched CubeMX board tree (generated)

This directory is produced by `scripts/sync-cubemx-gambos-pcb.sh`.

Do not edit by hand — change `gambos-pcb_upstream/` and/or `gambos-pcb_patches/` instead, then re-run sync.
EOF

echo "OK: synced ${PATCHED} from upstream + ${#PATCHES[@]} patch(es)"
