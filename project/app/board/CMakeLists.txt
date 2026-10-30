target_sources(gambos_app PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/board.cpp
)

target_include_directories(gambos_app PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)
