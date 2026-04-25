add_library(custom_drivers STATIC)

target_include_directories(custom_drivers PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(nrf24l01p)

# CMSIS/HAL headers (GPIO_TypeDef, etc.) for drivers that use STM32 types.
target_link_libraries(custom_drivers PUBLIC stm32cubemx)