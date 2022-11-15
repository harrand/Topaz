cmake_minimum_required(VERSION 3.24)
add_subdirectory(vendor/HDK)
add_subdirectory(vendor/volk)
include(cmake/project.cmake)
project(tz VERSION 4.0.0.1)

hdk_add_library(TARGET tz)
target_sources(tz PRIVATE
	src/tz/tz.cpp
	src/tz/tz.hpp
)

message(STATUS "Topaz - Render API: \"${TOPAZ_RENDER_API}\"")
target_compile_definitions(tz PRIVATE "-DTZ_VER=0x${PROJECT_VERSION_MAJOR}${PROJECT_VERSION_MINOR}${PROJECT_VERSION_PATCH}${PROJECT_VERSION_TWEAK}")
if(${TOPAZ_RENDER_API} STREQUAL "Vulkan")
	target_link_libraries(tz PRIVATE volk)
	target_sources(tz PRIVATE
		src/impl/vulkan/tzvk.cpp
		src/impl/vulkan/tzvk.hpp
	)
	target_compile_options(tz PRIVATE "-DTOPAZ_VULKAN=1")
else()
	message(FATAL_ERROR "Unknown RenderAPI \"${TOPAZ_RENDER_API}\"")
	target_compile_options(tz PRIVATE "-DTOPAZ_VULKAN=0")
endif()

enable_testing()
add_subdirectory(test)

target_include_directories(tz PUBLIC ${PROJECT_SOURCE_DIR}/src)
