cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME gambos)

# Board selection: custom/devkit = F446 symmetric layout (see board/README.md)
set(BOARD "custom" CACHE STRING "Target board: custom or devkit")
if(NOT BOARD MATCHES "^(custom|devkit)$")
    message(FATAL_ERROR "BOARD must be 'custom' or 'devkit', got: ${BOARD}")
endif()
message(STATUS "Board: ${BOARD}")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings (C++17 for app code; HAL/Cube stays C)
project(${CMAKE_PROJECT_NAME} LANGUAGES C CXX ASM)
message("Build type: " ${CMAKE_BUILD_TYPE})

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Custom device drivers (static lib; link into firmware below)
add_subdirectory(custom_drivers)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Board-specific application (custom = PCB app; devkit = app/devkit/CMakeLists.txt)
if(BOARD STREQUAL "devkit")
    add_subdirectory(app/devkit)
else()
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE app/${BOARD}/app.c)
endif()

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/app/${BOARD}
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries (plain signature — must match stm32cubemx / stm32f4_freertos.cmake)
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    custom_drivers
)
