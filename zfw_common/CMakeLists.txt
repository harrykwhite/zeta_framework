project(zfw_common)

add_library(zfw_common STATIC
    src/zfw_common_debug.c
    src/zfw_common_mem.c
    src/zfw_common_math.c

    include/zfw_common.h
)

target_include_directories(zfw_common PRIVATE include)
