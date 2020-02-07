cmake_minimum_required(VERSION 3.9)
project(topaz VERSION 0.0.1 DESCRIPTION "Cross-Platform Multimedia Engine")
# CMake Variable Redefinitions
set(CMAKE_BINARY_DIR ${PROJECT_SOURCE_DIR}/build)
set(CMAKE_CXX_STANDARD 17)
# Topaz-Specific Definitions
set(TOPAZ_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)

add_library(topaz STATIC
		src/core/debug/assert.hpp
		src/core/debug/print.hpp
		src/core/debug/print.inl
		src/core/core.hpp
		src/core/core.cpp
		src/core/core.inl
		src/core/resource_manager.cpp
		src/core/resource_manager.hpp
		src/core/tz_glfw/glfw_context.cpp
		src/core/tz_glfw/glfw_context.hpp
		src/core/tz_glfw/glfw_window.cpp
		src/core/tz_glfw/glfw_window.hpp
		src/core/window.cpp
		src/core/window.hpp
		src/core/window.inl
		src/core/tz_glad/glad_context.cpp
		src/core/tz_glad/glad_context.hpp
		src/input/input_listener.cpp
		src/input/input_listener.hpp
		src/input/input_event.cpp
		src/input/input_event.hpp
		src/input/system_input.cpp
		src/input/system_input.hpp
		src/memory/align.hpp
		src/memory/align.inl
		src/memory/block.cpp
		src/memory/block.hpp
		src/memory/pool.hpp
		src/memory/pool.inl
		src/geo/matrix_transform.cpp
		src/geo/matrix_transform.hpp
		src/geo/matrix.hpp
		src/geo/matrix.inl
		src/geo/vector.hpp
		src/geo/vector.inl
		src/gl/tz_stb_image/image_reader.hpp
		src/gl/tz_stb_image/image_reader.inl
		src/gl/tz_stb_image/image_writer.hpp
		src/gl/tz_stb_image/image_writer.inl
		src/gl/buffer.cpp
		src/gl/buffer.hpp
		src/gl/buffer.inl
		src/gl/draw_command.hpp
		src/gl/draw_command.cpp
		src/gl/format.hpp
		src/gl/frame.cpp
		src/gl/frame.hpp
		src/gl/frame.inl
		src/gl/image.cpp
		src/gl/image.hpp
		src/gl/object.hpp
		src/gl/object.cpp
		src/gl/object.inl
		src/gl/pixel.hpp
		src/gl/pixel.inl
		src/gl/shader.cpp
		src/gl/shader.hpp
		src/gl/shader.inl
		src/gl/shader_compiler.cpp
		src/gl/shader_compiler.hpp
		src/gl/shader_preprocessor.cpp
		src/gl/shader_preprocessor.hpp
		src/gl/shader_preprocessor.inl
		src/gl/texture.cpp
		src/gl/texture.hpp
		src/gl/texture.inl
		src/gl/modules/include.cpp
		src/gl/modules/include.hpp
		src/gl/modules/ssbo.cpp
		src/gl/modules/ssbo.hpp
		src/gl/modules/ubo.cpp
		src/gl/modules/ubo.hpp
		src/render/device.cpp
		src/render/device.hpp
		src/render/index_snippet.cpp
		src/render/index_snippet.hpp
		src/render/pipeline.cpp
		src/render/pipeline.hpp)

target_link_libraries(topaz PUBLIC
		glfw
		glad
		stbi)
target_include_directories(topaz PUBLIC ${TOPAZ_SOURCE_DIR})

add_subdirectory(lib)
add_subdirectory(demo)
add_subdirectory(test)

set_target_properties(topaz
		PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
		LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
		)

# Effects *everything*
add_compile_options(-Wall -Wextra -pedantic-errors)

# Topaz Compile Options

## Build-config specific
if(${TOPAZ_DEBUG})
	message(STATUS "Topaz Debug")
	target_compile_options(topaz PUBLIC -O0 -g)
	target_compile_definitions(topaz PUBLIC -DTOPAZ_DEBUG=1 -DTOPAZ_RELEASE=0)
else()
	message(STATUS "Topaz Release")
	target_compile_options(topaz PUBLIC -O3 -fno-exceptions)
	target_compile_definitions(topaz PUBLIC -DTOPAZ_DEBUG=0 -DTOPAZ_RELEASE=1)
endif()

## General
target_compile_options(topaz PRIVATE -fno-rtti) # Disable rrti (until c++20 provides better rtti)
target_compile_options(topaz PRIVATE -fstrict-aliasing -msse4.1) # Enable strict aliasing (which we refuse to violate) for a small performance boost.
# Unless overriden, we aren't compiling in unit-test mode.
target_compile_definitions(topaz PUBLIC -DTOPAZ_UNIT_TEST=0)
target_compile_definitions(topaz PUBLIC -DTOPAZ_PROJECT_DIR=\"${PROJECT_SOURCE_DIR}\")
