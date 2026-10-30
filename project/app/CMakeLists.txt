# Gambos PCB application sources, built as their own object library so
# project warning flags apply here and not to the Cube-generated sources.
# Invoked from top-level CMakeLists.txt via add_subdirectory(app).

add_library(gambos_app OBJECT)

target_include_directories(gambos_app PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(gambos_app PUBLIC
    stm32cubemx
    little_fs
    fat_fs
    custom_drivers
)

target_sources(gambos_app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/app/app.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/retarget_print.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtos_debug.cpp
)

add_subdirectory(button_handler)
add_subdirectory(board)
add_subdirectory(delayable_handler)
add_subdirectory(messaging)
add_subdirectory(sensing_handler)
add_subdirectory(storage_handler)
add_subdirectory(actuator_handler)
