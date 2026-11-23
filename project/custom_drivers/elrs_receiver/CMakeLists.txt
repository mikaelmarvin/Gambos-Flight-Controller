target_sources(custom_drivers PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/elrs_receiver.cpp
)

target_include_directories(custom_drivers PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)
