#!/usr/bin/env bash
# Regenerate board/gambos-pcb_patches/*.patch from upstream vs current patched tree.
#
# Usage:
#   ./software/project/scripts/regenerate-board-patches.sh           # all managed patches
#   ./software/project/scripts/regenerate-board-patches.sh 001 005   # only these
#   ./software/project/scripts/regenerate-board-patches.sh 001-main-app-hooks
#
# Only rewrites a .patch file when its hunks actually change (no timestamp noise).
# Run after editing board/gambos-pcb/ by hand. Review git diff before committing.
#
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BOARD="gambos-pcb"
UPSTREAM="${ROOT}/board/${BOARD}_upstream"
PATCHED="${ROOT}/board/${BOARD}"
PATCH_DIR="${ROOT}/board/${BOARD}_patches"

# One patch ↔ one file (except 004+008, which stack on stm32f4xx_it.c).
declare -a REL_PATHS=(
    "Core/Src/main.c"
    "Core/Src/freertos.c"
    "Core/Src/gpio.c"
    "Core/Src/tim.c"
    "CMakeLists.txt"
    "Core/Src/i2c.c"
)

declare -a PATCH_NAMES=(
    "001-main-app-hooks"
    "002-freertos-default-task"
    "003-gpio-button-exti"
    "005-tim-pwm-tuning"
    "006-cubeide-cmake-note"
    "007-i2c1-msp-nvic"
)

# Compare hunks only: drop ---/+++ path + timestamp noise.
normalize_patch() {
    sed -E 's/^(---|\+\+\+)[[:space:]].*/\1 FILE/'
}

# Write tmp patch to out only if normalized content differs.
write_patch_if_changed() {
    local out="$1"
    local tmp="$2"

    if [[ -f "${out}" ]] &&
        diff -q <(normalize_patch < "${out}") <(normalize_patch < "${tmp}") >/dev/null 2>&1; then
        rm -f "${tmp}"
        echo "Unchanged ${out}"
        return 0
    fi

    mv "${tmp}" "${out}"
    echo "Wrote ${out}"
}

# Diff left → right into tmp file path $4. Stable labels (no timestamps).
make_file_diff() {
    local rel="$1"
    local left="$2"
    local right="$3"
    local dest="$4"
    local rc=0
    diff -u --label "${rel}" --label "${rel}" "${left}" "${right}" > "${dest}" || rc=$?
    if [[ "${rc}" -gt 1 ]]; then
        return 1
    fi
    return 0
}

regenerate_simple() {
    local rel="$1"
    local name="$2"
    local out="${PATCH_DIR}/${name}.patch"
    local tmp

    if [[ ! -f "${UPSTREAM}/${rel}" ]]; then
        echo "Missing upstream file: ${UPSTREAM}/${rel}" >&2
        return 1
    fi
    if [[ ! -f "${PATCHED}/${rel}" ]]; then
        echo "Missing patched file: ${PATCHED}/${rel}" >&2
        return 1
    fi

    tmp="$(mktemp)"
    make_file_diff "${rel}" "${UPSTREAM}/${rel}" "${PATCHED}/${rel}" "${tmp}"
    if [[ ! -s "${tmp}" ]]; then
        echo "WARN: ${rel} identical upstream vs patched — ${name}.patch would be empty" >&2
    fi
    write_patch_if_changed "${out}" "${tmp}"
}

# 004 then 008 both edit stm32f4xx_it.c. Rebuild the pair from final patched file
# by reverse-applying 008 to recover the mid state (after 004 only).
regenerate_it_pair() {
    local rel="Core/Src/stm32f4xx_it.c"
    local out004="${PATCH_DIR}/004-stm32f4xx-it-exti0.patch"
    local out008="${PATCH_DIR}/008-i2c1-it-handlers.patch"
    local tmpdir mid t004 t008

    if [[ ! -f "${UPSTREAM}/${rel}" || ! -f "${PATCHED}/${rel}" ]]; then
        echo "Missing ${rel} in upstream or patched tree" >&2
        return 1
    fi
    if [[ ! -f "${out008}" ]]; then
        echo "Missing ${out008} (needed to split stacked it.c patches)" >&2
        return 1
    fi

    tmpdir="$(mktemp -d)"
    mid="${tmpdir}/${rel}"
    mkdir -p "$(dirname "${mid}")"
    cp "${PATCHED}/${rel}" "${mid}"

    if ! patch -d "${tmpdir}" -p0 -R --forward --no-backup-if-mismatch \
            --reject-file=- < "${out008}" >/dev/null; then
        rm -rf "${tmpdir}"
        echo "Failed to reverse-apply 008 onto patched ${rel}; edit 004/008 by hand" >&2
        return 1
    fi

    t004="$(mktemp)"
    t008="$(mktemp)"
    make_file_diff "${rel}" "${UPSTREAM}/${rel}" "${mid}" "${t004}"
    make_file_diff "${rel}" "${mid}" "${PATCHED}/${rel}" "${t008}"
    rm -rf "${tmpdir}"

    write_patch_if_changed "${out004}" "${t004}"
    write_patch_if_changed "${out008}" "${t008}"
}

should_run() {
    local name="$1"
    if [[ ${#REQUESTED[@]} -eq 0 ]]; then
        return 0
    fi
    local req
    for req in "${REQUESTED[@]}"; do
        if [[ "${req}" == "${name}" ||
              "${req}" == "${name%%-*}" ||
              "${name}" == *"${req}"* ]]; then
            return 0
        fi
    done
    return 1
}

REQUESTED=()
if [[ $# -gt 0 ]]; then
    REQUESTED=("$@")
fi

mkdir -p "${PATCH_DIR}"

if [[ ! -d "${UPSTREAM}" ]]; then
    echo "Missing upstream: ${UPSTREAM}" >&2
    exit 1
fi
if [[ ! -d "${PATCHED}" ]]; then
    echo "Missing patched tree: ${PATCHED}" >&2
    echo "Run: ./software/project/scripts/sync-cubemx-gambos-pcb.sh" >&2
    exit 1
fi

ran=0
for i in "${!REL_PATHS[@]}"; do
    name="${PATCH_NAMES[$i]}"
    if should_run "${name}"; then
        regenerate_simple "${REL_PATHS[$i]}" "${name}"
        ran=$((ran + 1))
    fi
done

if should_run "004-stm32f4xx-it-exti0" || should_run "008-i2c1-it-handlers" ||
    should_run "004" || should_run "008" || should_run "stm32f4xx_it"; then
    regenerate_it_pair
    ran=$((ran + 1))
fi

if [[ ${#REQUESTED[@]} -gt 0 && "${ran}" -eq 0 ]]; then
    echo "No matching patches for: ${REQUESTED[*]}" >&2
    echo "Try a name like: 001  001-main-app-hooks  005-tim-pwm-tuning" >&2
    exit 1
fi

echo "OK: regenerate done"
