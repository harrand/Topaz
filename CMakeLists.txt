cmake_minimum_required(VERSION 3.24)
add_subdirectory(vendor/HDK)
add_subdirectory(vendor/volk)
project(topaz VERSION 4.0)

hdk_add_library(TARGET topaz
	SOURCES
		src/tz/tz.cpp
		src/tz/tz.hpp
)

target_compile_options(topaz PUBLIC "-DTOPAZ_RENDER_API=${TOPAZ_RENDER_API}")
message(STATUS "Topaz - Render API: \"${TOPAZ_RENDER_API}\"")
if(${TOPAZ_RENDER_API} STREQUAL "Vulkan")
	target_link_libraries(topaz PRIVATE volk)
	target_sources(topaz PRIVATE
		src/impl/vulkan/tzvk.cpp
		src/impl/vulkan/tzvk.hpp
	)
else()
	message(FATAL_ERROR "Unknown RenderAPI \"${TOPAZ_RENDER_API}\"")
endif()


target_include_directories(topaz PUBLIC ${PROJECT_SOURCE_DIR}/src)
