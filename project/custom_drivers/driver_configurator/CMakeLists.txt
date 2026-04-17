target_sources(custom_drivers PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/driver_configurator.cpp
)
target_include_directories(custom_drivers PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)