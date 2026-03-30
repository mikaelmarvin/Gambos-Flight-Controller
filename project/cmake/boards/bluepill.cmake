# STM32F103 (Cortex-M3) — self-contained CubeMX tree under board/bluepill/
get_filename_component(_PROJ_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(TARGET_FLAGS "-mcpu=cortex-m3 -mthumb")
set(GAMBOARD_LINKER_SCRIPT "${_PROJ_ROOT}/board/bluepill/STM32F103XX_FLASH.ld")
