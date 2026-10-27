add_library(little_fs STATIC
    lfs.c
    lfs_util.c
)

target_include_directories(little_fs PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(little_fs PUBLIC LFS_NO_MALLOC)
