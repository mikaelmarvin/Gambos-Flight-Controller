#!/usr/bin/env bash
# Generate board-specific CMake inputs for the top-level build.
# Usage: ./software/project/scripts/gen-board-sources.sh <board> [generated-dir]
set -euo pipefail

BOARD="${1:-}"
if [[ -z "${BOARD}" ]]; then
    echo "Usage: $0 <board> [generated-dir]" >&2
    exit 2
fi

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
GEN_DIR="${2:-${ROOT}/build/${BOARD}/generated}"
CUBE_STM32CMAKE="${ROOT}/board/${BOARD}/cmake/stm32cubemx/CMakeLists.txt"
CUBE_TOOLCHAIN="${ROOT}/board/${BOARD}/cmake/gcc-arm-none-eabi.cmake"

if [[ ! -f "${CUBE_STM32CMAKE}" ]]; then
    echo "Missing Cube-generated file: ${CUBE_STM32CMAKE}" >&2
    exit 1
fi
if [[ ! -f "${CUBE_TOOLCHAIN}" ]]; then
    echo "Missing Cube-generated toolchain: ${CUBE_TOOLCHAIN}" >&2
    exit 1
fi

mkdir -p "${GEN_DIR}"

python3 - "${ROOT}" "${BOARD}" "${CUBE_STM32CMAKE}" "${CUBE_TOOLCHAIN}" "${GEN_DIR}" <<'PY'
import re
import sys
from pathlib import Path

root = Path(sys.argv[1])
board = sys.argv[2]
cube_stm32 = Path(sys.argv[3])
cube_toolchain = Path(sys.argv[4])
gen_dir = Path(sys.argv[5])

stm32_text = cube_stm32.read_text(encoding="utf-8")
toolchain_text = cube_toolchain.read_text(encoding="utf-8")

wanted = [
    "MX_Defines_Syms",
    "MX_Include_Dirs",
    "MX_Application_Src",
    "STM32_Drivers_Src",
    "FreeRTOS_Src",
    "MX_LINK_DIRS",
    "MX_LINK_LIBS",
]

def parse_set_block(text: str, name: str):
    m = re.search(rf"set\(\s*{re.escape(name)}\s*(.*?)\n\)", text, re.S)
    if not m:
        raise RuntimeError(f"Missing set({name} ...) in {cube_stm32}")
    values = []
    for raw in m.group(1).splitlines():
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        line = line.split("#", 1)[0].strip().replace("\r", "")
        if not line:
            continue
        line = line.replace(
            "${CMAKE_CURRENT_SOURCE_DIR}/../../",
            f"${{CMAKE_SOURCE_DIR}}/board/{board}/",
        )
        values.append(line)
    return values

blocks = {name: parse_set_block(stm32_text, name) for name in wanted}

# Validate rewritten paths.
for key in ["MX_Include_Dirs", "MX_Application_Src", "STM32_Drivers_Src", "FreeRTOS_Src"]:
    for item in blocks[key]:
        if not item.startswith("${CMAKE_SOURCE_DIR}/board/"):
            continue
        rel = item.replace("${CMAKE_SOURCE_DIR}/", "", 1)
        p = root / rel
        if not p.exists():
            raise RuntimeError(f"Path from {key} does not exist: {p}")

target_flags_match = re.search(r"set\(TARGET_FLAGS\s+\"([^\"]+)\"", toolchain_text)
if not target_flags_match:
    raise RuntimeError(f"Could not parse TARGET_FLAGS from {cube_toolchain}")
target_flags = target_flags_match.group(1).strip()

ld_match = re.search(r"-T\s+\\\"?\$\{CMAKE_SOURCE_DIR\}/([^\"\\\s]+\.ld)", toolchain_text)
if not ld_match:
    ld_match = re.search(r"-T\s+\"?\$\{CMAKE_SOURCE_DIR\}/([^\"\s]+\.ld)", toolchain_text)
if not ld_match:
    raise RuntimeError(f"Could not parse linker script path from {cube_toolchain}")
ld_name = ld_match.group(1).split("/")[-1]
board_ld = root / "board" / board / ld_name
if not board_ld.exists():
    raise RuntimeError(f"Linker script from Cube toolchain not found at {board_ld}")

def emit_set(name: str):
    lines = [f"set({name}"]
    lines.extend([f"    {v}" for v in blocks[name]])
    lines.append(")")
    return "\n".join(lines)

paths_out = []
paths_out.append("# Auto-generated from board/<board>/cmake/stm32cubemx/CMakeLists.txt")
paths_out.append("# DO NOT EDIT: regenerate via scripts/gen-board-sources.sh")
paths_out.append(f"set(CUBEMX_BOARD \"{board}\")")
paths_out.append("")
for name in wanted:
    paths_out.append(emit_set(name))
    paths_out.append("")
paths_content = "\n".join(paths_out).rstrip() + "\n"

toolchain_out = f"""# Auto-generated from board/<board>/cmake/gcc-arm-none-eabi.cmake
# DO NOT EDIT: regenerate via scripts/gen-board-sources.sh
set(CMAKE_SYSTEM_NAME               Generic)
set(CMAKE_SYSTEM_PROCESSOR          arm)

set(CMAKE_C_COMPILER_ID GNU)
set(CMAKE_CXX_COMPILER_ID GNU)

set(TOOLCHAIN_PREFIX                arm-none-eabi-)

set(CMAKE_C_COMPILER                ${{TOOLCHAIN_PREFIX}}gcc)
set(CMAKE_ASM_COMPILER              ${{CMAKE_C_COMPILER}})
set(CMAKE_CXX_COMPILER              ${{TOOLCHAIN_PREFIX}}g++)
set(CMAKE_LINKER                    ${{TOOLCHAIN_PREFIX}}g++)
set(CMAKE_OBJCOPY                   ${{TOOLCHAIN_PREFIX}}objcopy)
set(CMAKE_SIZE                      ${{TOOLCHAIN_PREFIX}}size)

set(CMAKE_EXECUTABLE_SUFFIX_ASM     ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_C       ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX     ".elf")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(TARGET_FLAGS "{target_flags}")
set(GAMBOARD_LINKER_SCRIPT "${{CMAKE_SOURCE_DIR}}/board/{board}/{ld_name}")

set(CMAKE_C_FLAGS "${{CMAKE_C_FLAGS}} ${{TARGET_FLAGS}}")
set(CMAKE_ASM_FLAGS "${{CMAKE_C_FLAGS}} -x assembler-with-cpp -MMD -MP")
set(CMAKE_C_FLAGS "${{CMAKE_C_FLAGS}} -Wall -fdata-sections -ffunction-sections")

set(CMAKE_C_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_C_FLAGS_RELEASE "-Os -g0")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_CXX_FLAGS_RELEASE "-Os -g0")

set(CMAKE_CXX_FLAGS "${{CMAKE_C_FLAGS}} -fno-rtti -fno-exceptions -fno-threadsafe-statics")

set(CMAKE_EXE_LINKER_FLAGS "${{TARGET_FLAGS}}")
set(CMAKE_EXE_LINKER_FLAGS "${{CMAKE_EXE_LINKER_FLAGS}} -T \\"${{GAMBOARD_LINKER_SCRIPT}}\\"")
set(CMAKE_EXE_LINKER_FLAGS "${{CMAKE_EXE_LINKER_FLAGS}} --specs=nano.specs")
set(CMAKE_EXE_LINKER_FLAGS "${{CMAKE_EXE_LINKER_FLAGS}} -Wl,-Map=${{CMAKE_PROJECT_NAME}}.map -Wl,--gc-sections")
set(CMAKE_EXE_LINKER_FLAGS "${{CMAKE_EXE_LINKER_FLAGS}} -Wl,--print-memory-usage")
set(TOOLCHAIN_LINK_LIBRARIES "m")
"""

for path, content in [
    (gen_dir / "cubemx_paths.cmake", paths_content),
    (gen_dir / "toolchain.cmake", toolchain_out),
]:
    if path.exists() and path.read_text(encoding="utf-8") == content:
        continue
    path.write_text(content, encoding="utf-8")
PY

echo "Generated: ${GEN_DIR}/cubemx_paths.cmake"
echo "Generated: ${GEN_DIR}/toolchain.cmake"
