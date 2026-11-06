target_sources(gambos_app_lib PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/board.cpp
)

target_include_directories(gambos_app_lib PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)
