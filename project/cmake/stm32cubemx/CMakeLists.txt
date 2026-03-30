cmake_minimum_required(VERSION 3.22)
enable_language(C ASM)

if(BOARD STREQUAL "bluepill")
    include(${CMAKE_CURRENT_LIST_DIR}/stm32f1_bluepill_freertos.cmake)
else()
    include(${CMAKE_CURRENT_LIST_DIR}/stm32f4_freertos.cmake)
endif()
