project(zfw_asset_packer)

find_package(cJSON CONFIG REQUIRED)
find_package(Freetype REQUIRED)

get_filename_component(PARENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PATH)

add_executable(zfw_asset_packer
	src/main.c

	${PARENT_SOURCE_DIR}/vendor/stb_image/include/stb_image.h
	${PARENT_SOURCE_DIR}/vendor/stb_image/src/stb_image.c
)

target_include_directories(zfw_asset_packer PRIVATE
	include
	${PARENT_SOURCE_DIR}/zfw_common/include
	${PARENT_SOURCE_DIR}/vendor/stb_image/include
)

target_link_libraries(zfw_asset_packer PRIVATE zfw_common cjson Freetype::Freetype)
