#!/usr/bin/env bash
# Regenerate board/gambos-pcb_patches/*.patch from upstream vs current patched tree.
#
# Usage:
#   ./software/project/scripts/regenerate-board-patches.sh
#
# Run after editing board/gambos-pcb/ by hand to capture new customizations.
# Review the generated patch diff before committing.
#
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BOARD="gambos-pcb"
UPSTREAM="${ROOT}/board/${BOARD}_upstream"
PATCHED="${ROOT}/board/${BOARD}"
PATCH_DIR="${ROOT}/board/${BOARD}_patches"

declare -a REL_PATHS=(
    "Core/Src/main.c"
    "Core/Src/freertos.c"
    "Core/Src/gpio.c"
    "Core/Src/stm32f4xx_it.c"
    "Core/Src/tim.c"
    "CMakeLists.txt"
)

declare -a PATCH_NAMES=(
    "001-main-app-hooks"
    "002-freertos-default-task"
    "003-gpio-button-exti"
    "004-stm32f4xx-it-exti0"
    "005-tim-pwm-tuning"
    "006-cubeide-cmake-note"
)

mkdir -p "${PATCH_DIR}"

for i in "${!REL_PATHS[@]}"; do
    rel="${REL_PATHS[$i]}"
    name="${PATCH_NAMES[$i]}"
    out="${PATCH_DIR}/${name}.patch"
    if [[ ! -f "${UPSTREAM}/${rel}" ]]; then
        echo "Missing upstream file: ${UPSTREAM}/${rel}" >&2
        exit 1
    fi
    if [[ ! -f "${PATCHED}/${rel}" ]]; then
        echo "Missing patched file: ${PATCHED}/${rel}" >&2
        exit 1
    fi
    diff -u "${UPSTREAM}/${rel}" "${PATCHED}/${rel}" \
        | sed "s|${UPSTREAM}/||g; s|${PATCHED}/||g" > "${out}" || {
        if [[ ${PIPESTATUS[0]} -gt 1 ]]; then
            exit 1
        fi
    }
    echo "Wrote ${out}"
done

echo "OK: regenerated ${#REL_PATHS[@]} patch file(s)"
