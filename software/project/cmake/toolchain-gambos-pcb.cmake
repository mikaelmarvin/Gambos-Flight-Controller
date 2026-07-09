# Top-level firmware toolchain for the Gambos PCB board target.
# Wraps Cube-generated board/gambos-pcb/cmake/gcc-arm-none-eabi.cmake and fixes
# the linker script path (Cube assumes the project root is board/gambos-pcb/).

set(BOARD "gambos-pcb")

include(${CMAKE_CURRENT_LIST_DIR}/../board/${BOARD}/cmake/gcc-arm-none-eabi.cmake)

set(GAMBOARD_LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/board/${BOARD}/STM32F446XX_FLASH.ld")
string(REGEX REPLACE
    "-T \"[^\"]+\""
    "-T \"${GAMBOARD_LINKER_SCRIPT}\""
    CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}"
)
