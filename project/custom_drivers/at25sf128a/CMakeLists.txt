target_sources(custom_drivers PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/at25sf128a.cpp
)

target_include_directories(custom_drivers PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)
