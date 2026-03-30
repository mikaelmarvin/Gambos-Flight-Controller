# STM32F103 Blue Pill + FreeRTOS — CubeMX tree under board/bluepill (Core/, Drivers/, Middlewares/)
# Source lists mirror board/bluepill/cmake/stm32cubemx/CMakeLists.txt after CubeMX regenerate.

set(BP_ROOT "${CMAKE_SOURCE_DIR}/board/bluepill")

set(MX_Defines_Syms
	USE_HAL_DRIVER
	STM32F103xB
    $<$<CONFIG:Debug>:DEBUG>
)

set(MX_Include_Dirs
    ${BP_ROOT}/Core/Inc
    ${BP_ROOT}/Drivers/STM32F1xx_HAL_Driver/Inc
    ${BP_ROOT}/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy
    ${BP_ROOT}/Middlewares/Third_Party/FreeRTOS/Source/include
    ${BP_ROOT}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
    ${BP_ROOT}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3
    ${BP_ROOT}/Drivers/CMSIS/Device/ST/STM32F1xx/Include
    ${BP_ROOT}/Drivers/CMSIS/Include
)

set(MX_Application_Src
    ${BP_ROOT}/Core/Src/main.c
    ${BP_ROOT}/Core/Src/gpio.c
    ${BP_ROOT}/Core/Src/freertos.c
    ${BP_ROOT}/Core/Src/spi.c
    ${BP_ROOT}/Core/Src/stm32f1xx_it.c
    ${BP_ROOT}/Core/Src/stm32f1xx_hal_msp.c
    ${BP_ROOT}/Core/Src/stm32f1xx_hal_timebase_tim.c
    ${BP_ROOT}/Core/Src/sysmem.c
    ${BP_ROOT}/Core/Src/syscalls.c
    ${BP_ROOT}/startup_stm32f103xb.s
)

set(STM32_Drivers_Src
    ${BP_ROOT}/Core/Src/system_stm32f1xx.c
    ${BP_ROOT}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio_ex.c
    ${BP_ROOT}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim.c
    ${BP_ROOT}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim_ex.c
    ${BP_ROOT}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c
    ${BP_ROOT}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc.c
    ${BP_ROOT}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc_ex.c
    ${BP_ROOT}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio.c
    ${BP_ROOT}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dma.c
    ${BP_ROOT}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cortex.c
    ${BP_ROOT}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pwr.c
    ${BP_ROOT}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash.c
    ${BP_ROOT}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash_ex.c
    ${BP_ROOT}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_exti.c
    ${BP_ROOT}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_spi.c
)

set(FreeRTOS_Src
    ${BP_ROOT}/Middlewares/Third_Party/FreeRTOS/Source/croutine.c
    ${BP_ROOT}/Middlewares/Third_Party/FreeRTOS/Source/event_groups.c
    ${BP_ROOT}/Middlewares/Third_Party/FreeRTOS/Source/list.c
    ${BP_ROOT}/Middlewares/Third_Party/FreeRTOS/Source/queue.c
    ${BP_ROOT}/Middlewares/Third_Party/FreeRTOS/Source/stream_buffer.c
    ${BP_ROOT}/Middlewares/Third_Party/FreeRTOS/Source/tasks.c
    ${BP_ROOT}/Middlewares/Third_Party/FreeRTOS/Source/timers.c
    ${BP_ROOT}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/cmsis_os2.c
    ${BP_ROOT}/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c
    ${BP_ROOT}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3/port.c
)

set(MX_LINK_DIRS
)

set(MX_LINK_LIBS 
    STM32_Drivers
    ${TOOLCHAIN_LINK_LIBRARIES}
    FreeRTOS
)

add_library(stm32cubemx INTERFACE)
target_include_directories(stm32cubemx INTERFACE ${MX_Include_Dirs})
target_compile_definitions(stm32cubemx INTERFACE ${MX_Defines_Syms})

add_library(STM32_Drivers OBJECT)
target_sources(STM32_Drivers PRIVATE ${STM32_Drivers_Src})
target_link_libraries(STM32_Drivers PUBLIC stm32cubemx)

add_library(FreeRTOS OBJECT)
target_sources(FreeRTOS PRIVATE ${FreeRTOS_Src})
target_link_libraries(FreeRTOS PUBLIC stm32cubemx)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${MX_Application_Src})

target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE ${MX_LINK_DIRS})

target_link_libraries(${CMAKE_PROJECT_NAME} ${MX_LINK_LIBS})

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES ${CMAKE_PROJECT_NAME}.map)

if((CMAKE_C_STANDARD EQUAL 90) OR (CMAKE_C_STANDARD EQUAL 99))
    message(ERROR "Generated code requires C11 or higher")
endif()
