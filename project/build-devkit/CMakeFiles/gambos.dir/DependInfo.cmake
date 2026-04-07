
# Consider dependencies only in project.
set(CMAKE_DEPENDS_IN_PROJECT_ONLY OFF)

# The set of languages for which implicit dependencies are needed:
set(CMAKE_DEPENDS_LANGUAGES
  "ASM"
  )
# The set of files for implicit dependencies of each language:
set(CMAKE_DEPENDS_CHECK_ASM
  "/workspace/project/gambos-sw/project/board/devkit/startup_stm32f446xx.s" "/workspace/project/gambos-sw/project/build-devkit/CMakeFiles/gambos.dir/board/devkit/startup_stm32f446xx.s.o"
  )
set(CMAKE_ASM_COMPILER_ID "GNU")

# Preprocessor definitions for this target.
set(CMAKE_TARGET_DEFINITIONS_ASM
  "DEBUG"
  "STM32F446xx"
  "USE_HAL_DRIVER"
  )

# The include file search paths:
set(CMAKE_ASM_TARGET_INCLUDE_PATH
  "../app/devkit"
  "../custom_drivers/nrf24"
  "../board/devkit/Core/Inc"
  "../cmake/stm32cubemx/../../Drivers/STM32F4xx_HAL_Driver/Inc"
  "../cmake/stm32cubemx/../../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy"
  "../cmake/stm32cubemx/../../Middlewares/Third_Party/FreeRTOS/Source/include"
  "../cmake/stm32cubemx/../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2"
  "../cmake/stm32cubemx/../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F"
  "../cmake/stm32cubemx/../../Drivers/CMSIS/Device/ST/STM32F4xx/Include"
  "../cmake/stm32cubemx/../../Drivers/CMSIS/Include"
  )

# The set of dependency files which are needed:
set(CMAKE_DEPENDS_DEPENDENCY_FILES
  "/workspace/project/gambos-sw/project/app/devkit/app.c" "CMakeFiles/gambos.dir/app/devkit/app.c.o" "gcc" "CMakeFiles/gambos.dir/app/devkit/app.c.o.d"
  "/workspace/project/gambos-sw/project/app/devkit/tx_handler/tx_handler.c" "CMakeFiles/gambos.dir/app/devkit/tx_handler/tx_handler.c.o" "gcc" "CMakeFiles/gambos.dir/app/devkit/tx_handler/tx_handler.c.o.d"
  "/workspace/project/gambos-sw/project/board/devkit/Core/Src/dma.c" "CMakeFiles/gambos.dir/board/devkit/Core/Src/dma.c.o" "gcc" "CMakeFiles/gambos.dir/board/devkit/Core/Src/dma.c.o.d"
  "/workspace/project/gambos-sw/project/board/devkit/Core/Src/freertos.c" "CMakeFiles/gambos.dir/board/devkit/Core/Src/freertos.c.o" "gcc" "CMakeFiles/gambos.dir/board/devkit/Core/Src/freertos.c.o.d"
  "/workspace/project/gambos-sw/project/board/devkit/Core/Src/gpio.c" "CMakeFiles/gambos.dir/board/devkit/Core/Src/gpio.c.o" "gcc" "CMakeFiles/gambos.dir/board/devkit/Core/Src/gpio.c.o.d"
  "/workspace/project/gambos-sw/project/board/devkit/Core/Src/main.c" "CMakeFiles/gambos.dir/board/devkit/Core/Src/main.c.o" "gcc" "CMakeFiles/gambos.dir/board/devkit/Core/Src/main.c.o.d"
  "/workspace/project/gambos-sw/project/board/devkit/Core/Src/spi.c" "CMakeFiles/gambos.dir/board/devkit/Core/Src/spi.c.o" "gcc" "CMakeFiles/gambos.dir/board/devkit/Core/Src/spi.c.o.d"
  "/workspace/project/gambos-sw/project/board/devkit/Core/Src/stm32f4xx_hal_msp.c" "CMakeFiles/gambos.dir/board/devkit/Core/Src/stm32f4xx_hal_msp.c.o" "gcc" "CMakeFiles/gambos.dir/board/devkit/Core/Src/stm32f4xx_hal_msp.c.o.d"
  "/workspace/project/gambos-sw/project/board/devkit/Core/Src/stm32f4xx_hal_timebase_tim.c" "CMakeFiles/gambos.dir/board/devkit/Core/Src/stm32f4xx_hal_timebase_tim.c.o" "gcc" "CMakeFiles/gambos.dir/board/devkit/Core/Src/stm32f4xx_hal_timebase_tim.c.o.d"
  "/workspace/project/gambos-sw/project/board/devkit/Core/Src/stm32f4xx_it.c" "CMakeFiles/gambos.dir/board/devkit/Core/Src/stm32f4xx_it.c.o" "gcc" "CMakeFiles/gambos.dir/board/devkit/Core/Src/stm32f4xx_it.c.o.d"
  "/workspace/project/gambos-sw/project/board/devkit/Core/Src/syscalls.c" "CMakeFiles/gambos.dir/board/devkit/Core/Src/syscalls.c.o" "gcc" "CMakeFiles/gambos.dir/board/devkit/Core/Src/syscalls.c.o.d"
  "/workspace/project/gambos-sw/project/board/devkit/Core/Src/sysmem.c" "CMakeFiles/gambos.dir/board/devkit/Core/Src/sysmem.c.o" "gcc" "CMakeFiles/gambos.dir/board/devkit/Core/Src/sysmem.c.o.d"
  "/workspace/project/gambos-sw/project/board/devkit/Core/Src/usart.c" "CMakeFiles/gambos.dir/board/devkit/Core/Src/usart.c.o" "gcc" "CMakeFiles/gambos.dir/board/devkit/Core/Src/usart.c.o.d"
  )

# Targets to which this target links.
set(CMAKE_TARGET_LINKED_INFO_FILES
  )

# Fortran module output directory.
set(CMAKE_Fortran_TARGET_MODULE_DIR "")
