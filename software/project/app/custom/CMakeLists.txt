# Custom board application sources (linked when BOARD=custom).
# Invoked from top-level CMakeLists.txt via add_subdirectory(app/custom).

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/app/app.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/retarget_print.cpp
)

add_subdirectory(button_handler)
add_subdirectory(messaging)
add_subdirectory(tx_handler)
