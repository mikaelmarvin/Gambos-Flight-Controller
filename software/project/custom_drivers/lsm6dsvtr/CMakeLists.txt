target_sources(custom_drivers PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/lsm6dsvtr.cpp
)

target_include_directories(custom_drivers PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)
