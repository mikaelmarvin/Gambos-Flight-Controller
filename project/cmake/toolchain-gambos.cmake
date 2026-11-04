# Top-level firmware toolchain for the Gambos PCB board target.
# Wraps Cube-generated board/cmake/gcc-arm-none-eabi.cmake and fixes
# the linker script path (Cube assumes the project root is board/).

include(${CMAKE_CURRENT_LIST_DIR}/../board/cmake/gcc-arm-none-eabi.cmake)

set(GAMBOARD_LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/board/STM32F446XX_FLASH.ld")
string(REGEX REPLACE
    "-T \"[^\"]+\""
    "-T \"${GAMBOARD_LINKER_SCRIPT}\""
    CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}"
)

# Debug at -Og instead of Cube's -O0: still debuggable, but timing is
# much closer to optimized code and optimization-only bugs surface early.
set(CMAKE_C_FLAGS_DEBUG "-Og -g3")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -g3")
