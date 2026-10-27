add_library(fat_fs STATIC
    ff.c
)

target_include_directories(fat_fs PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)