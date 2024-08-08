project(zfw_common)

add_library(zfw_common STATIC
    src/zfw_common_debug.c
    src/zfw_common_mem.c
    src/zfw_common_bits.c
    src/zfw_common_math.c
    src/zfw_common_misc.c

    include/zfw_common_debug.h
    include/zfw_common_mem.h
    include/zfw_common_bits.h
    include/zfw_common_math.h
    include/zfw_common_assets.h
    include/zfw_common_misc.h
)

target_include_directories(zfw_common PRIVATE include)
