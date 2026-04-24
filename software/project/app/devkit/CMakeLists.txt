# Devkit application sources (linked when BOARD=devkit).
# Invoked from top-level CMakeLists.txt via add_subdirectory(app/devkit).

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/app/app.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/button_handler/button_handler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tx_handler/tx_handler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/retarget_print.cpp
)
