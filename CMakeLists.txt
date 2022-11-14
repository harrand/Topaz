cmake_minimum_required(VERSION 3.24)
add_subdirectory(vendor/HDK)
project(topaz VERSION 4.0)

hdk_add_library(TARGET topaz
	SOURCES
		src/tz/tz.hpp
		src/tz/tz.cpp
)

target_compile_options(topaz PUBLIC "-DTOPAZ_RENDER_API=${TOPAZ_RENDER_API}")
target_link_libraries(topaz PRIVATE volk)

target_include_directories(topaz PUBLIC ${PROJECT_SOURCE_DIR}/src)
