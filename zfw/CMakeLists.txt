project(zfw)

find_package(glfw3 CONFIG REQUIRED)

get_filename_component(PARENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PATH)

add_library(zfw STATIC
    src/zfw_game.c
    src/zfw_input.c
    src/zfw_assets.c
    src/zfw_rendering.c
    src/zfw_utils.c
    ${PARENT_SOURCE_DIR}/vendor/glad/src/glad.c

    include/zfw.h
    ${PARENT_SOURCE_DIR}/vendor/glad/include/glad/glad.h
    ${PARENT_SOURCE_DIR}/vendor/glad/include/KHR/khrplatform.h
)

target_include_directories(zfw PRIVATE
	include
	${PARENT_SOURCE_DIR}/zfw_common/include
	${PARENT_SOURCE_DIR}/vendor/glad/include
)

target_link_libraries(zfw PRIVATE zfw_common glfw)
