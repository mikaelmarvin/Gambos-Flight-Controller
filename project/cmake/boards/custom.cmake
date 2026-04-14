# STM32F446 — symmetric board/<custom>/{Inc,Src}; shared Drivers/ at project root
get_filename_component(_PROJ_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(TARGET_FLAGS "-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard")
set(GAMBOARD_LINKER_SCRIPT "${_PROJ_ROOT}/STM32F446XX_FLASH.ld")
