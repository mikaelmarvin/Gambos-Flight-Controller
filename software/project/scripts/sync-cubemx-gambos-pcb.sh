#!/usr/bin/env bash
# Materialize board/gambos-pcb from upstream Cube output + project patches.
#
# Usage:
#   ./software/project/scripts/sync-cubemx-gambos-pcb.sh
#
# Workflow after CubeMX generation:
#   1. Copy/replace board/gambos-pcb_upstream/ with the fresh Cube export
#   2. Run this script (normalizes CRLF→LF in upstream, then builds patched tree)
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

normalize_crlf_tree() {
    local tree="$1"
    export NORMALIZE_TREE="${tree}"
    python3 - <<'PY'
from pathlib import Path
root = Path(__import__('os').environ['NORMALIZE_TREE'])
exts = {'.c', '.h', '.s', '.txt', '.cmake', '.ioc', '.md', '.ld'}
converted = 0
for path in root.rglob('*'):
    if not path.is_file():
        continue
    if path.suffix not in exts and path.name not in ('CMakeLists.txt', '.mxproject'):
        continue
    data = path.read_bytes()
    if b'\r\n' in data:
        path.write_bytes(data.replace(b'\r\n', b'\n'))
        converted += 1
print(f"Normalized CRLF→LF in {converted} file(s) under {root}")
PY
}

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

# CubeMX (especially on Windows) writes CRLF. Repo / patches expect LF.
# Normalize upstream in place so git diffs stay about real Cube changes.
echo "Normalizing line endings in ${UPSTREAM}"
normalize_crlf_tree "${UPSTREAM}"

# Same formatter as editor format-on-save (clangd → clang-format).
# Only Core/ — not Drivers/ or Middlewares/ (vendor trees).
clang_format_core() {
    local tree="$1"
    if ! command -v "${CLANG_FORMAT}" >/dev/null 2>&1; then
        echo "WARN: ${CLANG_FORMAT} not found; skipping format" >&2
        return 0
    fi
    if [[ ! -f "${FORMAT_STYLE}" ]]; then
        echo "WARN: missing ${FORMAT_STYLE}; skipping format" >&2
        return 0
    fi
    local -a files=()
    while IFS= read -r -d '' f; do
        files+=("${f}")
    done < <(find "${tree}/Core" \( -name '*.c' -o -name '*.h' \) -print0 2>/dev/null)
    if [[ ${#files[@]} -eq 0 ]]; then
        return 0
    fi
    echo "Formatting ${#files[@]} Core file(s) with clang-format"
    "${CLANG_FORMAT}" -i -style="file:${FORMAT_STYLE}" "${files[@]}"
}

echo "Formatting upstream Core/ (matches format-on-save)"
clang_format_core "${UPSTREAM}"

echo "Sync ${BOARD}: ${UPSTREAM} -> ${PATCHED}"
rm -rf "${PATCHED}"
cp -a "${UPSTREAM}" "${PATCHED}"

for patch in "${PATCHES[@]}"; do
    echo "Applying $(basename "${patch}")"
    if ! patch -d "${PATCHED}" -p0 --forward --no-backup-if-mismatch \
            --reject-file=- < "${patch}"; then
        echo "Failed to apply patch: ${patch}" >&2
        exit 1
    fi
done

# Drop any *.orig leftovers from patch(1) (e.g. fuzzy applies / older patch).
find "${PATCHED}" -type f -name '*.orig' -delete
find "${PATCHED}" -type f -name '*.rej' -delete
# Re-format files touched by patches (patches may reintroduce Cube layout).
if command -v "${CLANG_FORMAT}" >/dev/null 2>&1 && [[ -f "${FORMAT_STYLE}" ]]; then
  mapfile -t PATCHED_FILES < <(awk '/^\+\+\+ / { sub(/^\+\+\+ /, ""); print }' "${PATCHES[@]}" | sort -u)
  for rel in "${PATCHED_FILES[@]}"; do
    file="${PATCHED}/${rel}"
    if [[ -f "${file}" ]]; then
      "${CLANG_FORMAT}" -i -style="file:${FORMAT_STYLE}" "${file}"
    fi
  done
fi

cat > "${PATCHED}/README.md" <<'EOF'
# Patched CubeMX board tree (generated)

This directory is produced by `scripts/sync-cubemx-gambos-pcb.sh`.

Do not edit by hand — change `gambos-pcb_upstream/` and/or `gambos-pcb_patches/` instead, then re-run sync.
EOF

echo "OK: synced ${PATCHED} from upstream + ${#PATCHES[@]} patch(es)"
