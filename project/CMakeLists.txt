cmake_minimum_required(VERSION 3.22)

#
# Gambos firmware (top-level orchestration).
# Board sources and Cube middleware targets come from
# board/cmake/stm32cubemx/ (add_subdirectory below).
#

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CMAKE_PROJECT_NAME gambos)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

project(${CMAKE_PROJECT_NAME} LANGUAGES C CXX ASM)
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

add_executable(${CMAKE_PROJECT_NAME})

set(CUBEMX_BOARD_CMAKE_DIR "${CMAKE_SOURCE_DIR}/board/cmake/stm32cubemx")
if(NOT EXISTS "${CUBEMX_BOARD_CMAKE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "Missing Cube board cmake: ${CUBEMX_BOARD_CMAKE_DIR}/CMakeLists.txt\n"
        "Expected committed tree at board/"
    )
endif()
add_subdirectory("${CUBEMX_BOARD_CMAKE_DIR}" cubemx_board)

add_subdirectory(third_party/little_fs)
add_subdirectory(third_party/fat_fs)
add_subdirectory(custom_drivers)
add_subdirectory(app)

# Warning flags for our own code only (app + custom_drivers). Cube, HAL,
# CMSIS and FreeRTOS headers are re-added as SYSTEM includes on these
# targets so warnings inside vendor headers are suppressed.
add_library(gambos_warnings INTERFACE)
target_compile_options(gambos_warnings INTERFACE
    -Wall
    -Wextra
    -Wshadow
    -Wconversion
    -Wdouble-promotion
)

get_target_property(MX_INCLUDE_DIRS stm32cubemx INTERFACE_INCLUDE_DIRECTORIES)
foreach(own_target gambos_app custom_drivers)
    target_include_directories(${own_target} SYSTEM PRIVATE ${MX_INCLUDE_DIRS})
    target_link_libraries(${own_target} PRIVATE gambos_warnings)
endforeach()

list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Keep the RTOS-aware debugging symbol (app/rtos_debug.cpp); nothing
# references it, so --gc-sections would otherwise drop it.
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    -Wl,--undefined=uxTopUsedPriority
)

target_link_libraries(${CMAKE_PROJECT_NAME}
    gambos_app
    stm32cubemx
    little_fs
    fat_fs
    custom_drivers
)
