target_sources(custom_drivers PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/bus.cpp
    ${CMAKE_CURRENT_LIST_DIR}/board_buses.cpp
)

target_include_directories(custom_drivers PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)
