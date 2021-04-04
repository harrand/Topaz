cmake_minimum_required(VERSION 3.9)
project(topaz VERSION 0.0.1 DESCRIPTION "Cross-Platform Multimedia Engine")
# CMake Variable Redefinitions
set(CMAKE_BINARY_DIR ${PROJECT_SOURCE_DIR}/build)
set(CMAKE_CXX_STANDARD 20)
# Topaz-Specific Definitions
set(TOPAZ_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)

add_library(topaz STATIC
		src/algo/container.hpp
		src/algo/container.inl
		src/algo/image_processing.hpp
		src/algo/image_processing.inl
		src/algo/math.cpp
		src/algo/math.hpp
		src/algo/math.inl
		src/algo/noise.hpp
		src/algo/noise.inl
		src/algo/physics_collisions.cpp
		src/algo/physics_collisions.hpp
		src/algo/static.hpp

		src/core/debug/assert.hpp
		src/core/debug/break.hpp
		src/core/debug/break.cpp
		src/core/debug/print.hpp
		src/core/debug/print.inl
		src/core/tz_glfw/interface.cpp
		src/core/tz_glfw/interface.inl
		src/core/tz_glfw/interface.hpp
		src/core/tz_glfw/window_functionality.cpp
		src/core/tz_glfw/window_functionality.hpp
		src/core/tz_glfw/window_impl.cpp
		src/core/tz_glfw/window_impl.hpp
		src/core/tz_glfw/settings.cpp
		src/core/tz_glfw/settings.hpp
		src/core/tz_glad/glad_context.cpp
		src/core/tz_glad/glad_context.hpp
		src/core/bool.hpp
		src/core/tz.hpp
		src/core/tz.cpp
		src/core/matrix_transform.cpp
		src/core/matrix_transform.hpp
		src/core/matrix.hpp
		src/core/matrix.inl
		src/core/quaternion.cpp
		src/core/quaternion.hpp
		src/core/random.cpp
		src/core/random.hpp
		src/core/random.inl
		src/core/resource_manager.cpp
		src/core/resource_manager.hpp
		src/core/scheduler.cpp
		src/core/scheduler.hpp
		src/core/settings.cpp
		src/core/settings.hpp
		src/core/time.cpp
		src/core/time.hpp
		src/core/types.hpp
		src/core/vector.hpp
		src/core/vector.inl
		src/core/window.cpp
		src/core/window.hpp
		src/core/window.inl

		src/input/input_listener.cpp
		src/input/input_listener.hpp
		src/input/input_event.cpp
		src/input/input_event.hpp
		src/input/system_input.cpp
		src/input/system_input.hpp

		src/memory/polymorphic/base.cpp
		src/memory/polymorphic/base.hpp
		src/memory/align.hpp
		src/memory/align.inl
		src/memory/block.cpp
		src/memory/block.hpp
		src/memory/polymorphic_variant.inl
		src/memory/polymorphic_variant.hpp
		src/memory/pool.hpp
		src/memory/pool.inl

		src/phys/colliders/generic.cpp
		src/phys/colliders/generic.hpp
		src/phys/colliders/sphere.cpp
		src/phys/colliders/sphere.hpp
		src/phys/resolvers/generic.cpp
		src/phys/resolvers/generic.hpp
		src/phys/resolvers/impulse.cpp
		src/phys/resolvers/impulse.hpp
		src/phys/resolvers/position.cpp
		src/phys/resolvers/position.hpp
		src/phys/body.cpp
		src/phys/body.hpp
		src/phys/collision.cpp
		src/phys/collision.hpp
		src/phys/motion.cpp
		src/phys/motion.hpp
		src/phys/world.cpp
		src/phys/world.hpp

		src/geo/boundary/aabb.cpp
		src/geo/boundary/aabb.hpp
		src/geo/boundary/sphere.cpp
		src/geo/boundary/sphere.hpp

		src/gl/tz_assimp/scene.hpp
		src/gl/tz_assimp/scene.cpp
		src/gl/tz_stb_image/image_reader.hpp
		src/gl/tz_stb_image/image_reader.inl
		src/gl/tz_stb_image/image_writer.hpp
		src/gl/tz_stb_image/image_writer.inl
		src/gl/buffer.hpp
		src/gl/buffer.cpp
		src/gl/buffer.inl
		src/gl/camera.hpp
		src/gl/camera.cpp
		src/gl/draw_command.hpp
		src/gl/draw_command.cpp
		src/gl/format.hpp
		src/gl/frame.cpp
		src/gl/frame.hpp
		src/gl/frame.inl
		src/gl/image.cpp
		src/gl/image.hpp
		src/gl/index_snippet.cpp
		src/gl/index_snippet.hpp
		src/gl/manager.hpp
		src/gl/manager.cpp
		src/gl/mesh_data.cpp
		src/gl/mesh_data.hpp
		src/gl/mesh_loader.hpp
		src/gl/mesh_loader.cpp
		src/gl/object.hpp
		src/gl/object.cpp
		src/gl/object.inl
		src/gl/pixel.hpp
		src/gl/pixel.inl
		src/gl/resource_writer.cpp
		src/gl/resource_writer.hpp
		src/gl/resource_writer.inl
		src/gl/screen.cpp
		src/gl/screen.hpp
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
		src/gl/texture_sentinel.cpp
		src/gl/texture_sentinel.hpp
		src/gl/transform.hpp
		src/gl/vertex.hpp
		src/gl/modules/bindless_sampler.cpp
		src/gl/modules/bindless_sampler.hpp
		src/gl/modules/cplvals.cpp
		src/gl/modules/cplvals.hpp
		src/gl/modules/include.cpp
		src/gl/modules/include.hpp
		src/gl/modules/ssbo.cpp
		src/gl/modules/ssbo.hpp
		src/gl/modules/ubo.cpp
		src/gl/modules/ubo.hpp

		src/dui/module_tabs/core_tab.cpp
		src/dui/module_tabs/core_tab.hpp
		src/dui/module_tabs/gl_tab.cpp
		src/dui/module_tabs/gl_tab.hpp
		src/dui/buffer_tracker.cpp
		src/dui/buffer_tracker.hpp
		src/dui/window.cpp
		src/dui/window.hpp
		src/dui/window.inl
		src/dui/imgui_impl_glfw.cpp
		src/dui/imgui_impl_glfw.h
		src/dui/imgui_impl_opengl3.cpp
		src/dui/imgui_impl_opengl3.h
		src/dui/ogl_info.cpp
		src/dui/ogl_info.hpp
		src/dui/texture_sentinel_tracker.cpp
		src/dui/texture_sentinel_tracker.hpp
		src/dui/tzglp_preview.cpp
		src/dui/tzglp_preview.hpp

		src/render/asset.cpp
		src/render/asset.hpp
		src/render/device.cpp
		src/render/device.hpp
		src/render/element_writer.hpp
		src/render/pipeline.cpp
		src/render/pipeline.hpp
		src/render/scene.cpp
		src/render/scene.hpp
		src/render/scene.inl)

target_link_libraries(topaz PUBLIC
		glfw
		glad
		stbi
		assimp
		imgui
		debugbreak)
target_include_directories(topaz PUBLIC ${TOPAZ_SOURCE_DIR})

set(GLAD_EXTENSIONS GL_ARB_bindless_texture)

add_subdirectory(lib)
add_subdirectory(demo)
add_subdirectory(test)
add_subdirectory(util)

set_target_properties(topaz
		PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
		LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
		)

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
target_compile_options(topaz PRIVATE -Wall -Wextra -pedantic-errors)
# Disabled warnings:
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	# For clang
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	# For gcc
	# -Wno-format -- GCC sucks at recognising %zu, which is plentiful in the code-base.
	target_compile_options(topaz PUBLIC -Wno-format)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
	# For icc
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	# For msvc
endif()

## General
target_compile_options(topaz PRIVATE -fno-rtti) # Disable rrti (until c++20 provides better rtti)
target_compile_options(topaz PRIVATE -fstrict-aliasing -msse4.1) # Enable strict aliasing (which we refuse to violate) for a small performance boost.
target_compile_options(topaz PUBLIC -fconcepts-diagnostics-depth=5) # Error diagnostics for concept constraint violation is very detailed
target_compile_definitions(topaz PUBLIC -DTOPAZ_PROJECT_DIR=\"${CMAKE_SOURCE_DIR}\" -DGLFW_INCLUDE_NONE)
