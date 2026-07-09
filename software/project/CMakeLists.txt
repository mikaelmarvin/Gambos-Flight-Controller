cmake_minimum_required(VERSION 3.22)

#
# Gambos firmware (top-level orchestration).
# Board sources and Cube middleware targets come from
# board/<BOARD>/cmake/stm32cubemx/ (add_subdirectory below).
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

if(NOT BOARD)
    set(BOARD "gambos-pcb" CACHE STRING "Target board")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

project(${CMAKE_PROJECT_NAME} LANGUAGES C CXX ASM)
message(STATUS "Board: ${BOARD}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

add_executable(${CMAKE_PROJECT_NAME})

set(CUBEMX_BOARD_CMAKE_DIR "${CMAKE_SOURCE_DIR}/board/${BOARD}/cmake/stm32cubemx")
if(NOT EXISTS "${CUBEMX_BOARD_CMAKE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "Missing Cube board cmake: ${CUBEMX_BOARD_CMAKE_DIR}/CMakeLists.txt\n"
        "Run: ./scripts/sync-cubemx-gambos-pcb.sh"
    )
endif()
add_subdirectory("${CUBEMX_BOARD_CMAKE_DIR}" cubemx_board)

add_subdirectory(third_party/little_fs)
add_subdirectory(custom_drivers)
add_subdirectory(app/${BOARD})

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/app/${BOARD}
)

list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    little_fs
    custom_drivers
)
