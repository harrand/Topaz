cmake_minimum_required(VERSION 3.9)
include(cmake/setup_configs.cmake)

file(READ ./version.txt TZ_VERSION)
# Reconfigure on changes to ./version.txt
configure_file(./version.txt topaz_version.xml)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(TZ_CXX_STANDARD 20)

project(topaz VERSION ${TZ_VERSION})

add_library(topaz
	# tz::core
	src/core/containers/basic_list.hpp
	src/core/containers/enum_field.hpp
	src/core/containers/enum_field.inl
	src/core/containers/polymorphic_list.hpp
	src/core/containers/polymorphic_list.inl

	src/core/algorithms/static.hpp
	
	src/core/allocators/adapter.hpp
	src/core/allocators/fallback.hpp
	src/core/allocators/malloc.cpp
	src/core/allocators/malloc.hpp
	src/core/allocators/null.hpp
	src/core/allocators/stack.hpp
	src/core/allocators/stack.inl

	src/core/interfaces/cloneable.hpp

	src/core/peripherals/keyboard.cpp
	src/core/peripherals/keyboard.hpp
	src/core/peripherals/monitor.cpp
	src/core/peripherals/monitor.hpp
	src/core/peripherals/mouse.cpp
	src/core/peripherals/mouse.hpp

	src/core/profiling/zone.hpp

	src/core/assert.hpp
	src/core/callback.hpp
	src/core/engine_info.cpp
	src/core/engine_info.hpp
	src/core/engine_info.inl
	src/core/game_info.hpp
	src/core/handle.hpp
	src/core/matrix_transform.cpp
	src/core/matrix_transform.hpp
	src/core/matrix.hpp
	src/core/matrix.inl
	src/core/memory.hpp
	src/core/report.hpp
	src/core/time.cpp
	src/core/time.hpp
	src/core/tz.cpp
	src/core/tz.hpp
	src/core/tz.inl
	src/core/vector.hpp
	src/core/vector.inl
	src/core/version.cpp
	src/core/version.hpp
	src/core/version.inl
	src/core/window_functionality.cpp
	src/core/window_functionality.hpp
	src/core/window_functionality.inl
	src/core/window.cpp
	src/core/window.hpp

	# tz::gl (API)
	src/gl/api/component.hpp
	src/gl/api/device.hpp
	src/gl/api/output.hpp
	src/gl/api/renderer.hpp
	src/gl/api/resource.hpp

	# tz::gl
	src/gl/component.hpp
	src/gl/device.hpp
	src/gl/output.cpp
	src/gl/output.hpp
	src/gl/renderer.hpp
	src/gl/resource.cpp
	src/gl/resource.hpp
	src/gl/resource.inl
	src/gl/tz_gl.hpp

	# tz::gl (Common Impl)
	src/gl/impl/frontend/common/renderer.cpp
	src/gl/impl/frontend/common/renderer.hpp
	src/gl/impl/frontend/common/shader.cpp
	src/gl/impl/frontend/common/shader.hpp

	# tz::gl (Vulkan Frontend)
	src/gl/impl/frontend/vk2/component.cpp
	src/gl/impl/frontend/vk2/component.hpp
	src/gl/impl/frontend/vk2/convert.hpp
	src/gl/impl/frontend/vk2/convert.inl
	src/gl/impl/frontend/vk2/device.cpp
	src/gl/impl/frontend/vk2/device.hpp
	src/gl/impl/frontend/vk2/renderer.cpp
	src/gl/impl/frontend/vk2/renderer.hpp
	src/gl/impl/frontend/vk2/renderer.inl

	# tz::gl (OpenGL Frontend)
	src/gl/impl/frontend/ogl2/component.cpp
	src/gl/impl/frontend/ogl2/component.hpp
	src/gl/impl/frontend/ogl2/convert.inl
	src/gl/impl/frontend/ogl2/convert.hpp
	src/gl/impl/frontend/ogl2/device.cpp
	src/gl/impl/frontend/ogl2/device.hpp
	src/gl/impl/frontend/ogl2/renderer.cpp
	src/gl/impl/frontend/ogl2/renderer.hpp

	# tz::gl (Vulkan Backend 2)
	src/gl/impl/backend/vk2/buffer.cpp
	src/gl/impl/backend/vk2/buffer.hpp
	src/gl/impl/backend/vk2/command.cpp
	src/gl/impl/backend/vk2/command.hpp
	src/gl/impl/backend/vk2/descriptors.cpp
	src/gl/impl/backend/vk2/descriptors.hpp
	src/gl/impl/backend/vk2/extensions.hpp
	src/gl/impl/backend/vk2/features.hpp
	src/gl/impl/backend/vk2/fence.cpp
	src/gl/impl/backend/vk2/fence.hpp
	src/gl/impl/backend/vk2/fixed_function.cpp
	src/gl/impl/backend/vk2/fixed_function.hpp
	src/gl/impl/backend/vk2/framebuffer.cpp
	src/gl/impl/backend/vk2/framebuffer.hpp
	src/gl/impl/backend/vk2/gpu_mem.hpp
	src/gl/impl/backend/vk2/graphics_pipeline.cpp
	src/gl/impl/backend/vk2/graphics_pipeline.hpp
	src/gl/impl/backend/vk2/tz_vulkan.cpp
	src/gl/impl/backend/vk2/tz_vulkan.hpp
	src/gl/impl/backend/vk2/image.cpp
	src/gl/impl/backend/vk2/image.hpp
	src/gl/impl/backend/vk2/image_view.cpp
	src/gl/impl/backend/vk2/image_view.hpp
	src/gl/impl/backend/vk2/image_format.hpp
	src/gl/impl/backend/vk2/logical_device.cpp
	src/gl/impl/backend/vk2/logical_device.hpp
	src/gl/impl/backend/vk2/pipeline_layout.cpp
	src/gl/impl/backend/vk2/pipeline_layout.hpp
	src/gl/impl/backend/vk2/render_pass.cpp
	src/gl/impl/backend/vk2/render_pass.hpp
	src/gl/impl/backend/vk2/sampler.cpp
	src/gl/impl/backend/vk2/sampler.hpp
	src/gl/impl/backend/vk2/semaphore.cpp
	src/gl/impl/backend/vk2/semaphore.hpp
	src/gl/impl/backend/vk2/shader.cpp
	src/gl/impl/backend/vk2/shader.hpp
	src/gl/impl/backend/vk2/swapchain.cpp
	src/gl/impl/backend/vk2/swapchain.hpp
	src/gl/impl/backend/vk2/hardware/physical_device.cpp
	src/gl/impl/backend/vk2/hardware/physical_device.hpp
	src/gl/impl/backend/vk2/hardware/queue.cpp
	src/gl/impl/backend/vk2/hardware/queue.hpp

	src/gl/impl/backend/vk2/vma.cpp

	# tz::gl (OpenGL Backend)
	src/gl/impl/backend/ogl2/buffer.cpp
	src/gl/impl/backend/ogl2/buffer.hpp
	src/gl/impl/backend/ogl2/framebuffer.cpp
	src/gl/impl/backend/ogl2/framebuffer.hpp
	src/gl/impl/backend/ogl2/image.cpp
	src/gl/impl/backend/ogl2/image.hpp
	src/gl/impl/backend/ogl2/image_format.hpp
	src/gl/impl/backend/ogl2/sampler.hpp
	src/gl/impl/backend/ogl2/shader.cpp
	src/gl/impl/backend/ogl2/shader.hpp
	src/gl/impl/backend/ogl2/tz_opengl.cpp
	src/gl/impl/backend/ogl2/tz_opengl.hpp
	src/gl/impl/backend/ogl2/vertex_array.cpp
	src/gl/impl/backend/ogl2/vertex_array.hpp
	)


message(STATUS "Topaz v${TZ_VERSION}")
set_target_properties(topaz PROPERTIES
	CXX_STANDARD ${TZ_CXX_STANDARD}
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF)


if(${TOPAZ_OGL})
	# Vulkan + OGL
	message(STATUS "RenderAPI: OpenGL (\"OGL\")")
	configure_opengl(topaz)
elseif(${TOPAZ_VULKAN})
	# Vulkan
	message(STATUS "RenderAPI: Vulkan (\"VK\")")
	configure_vulkan(topaz)
endif()
target_compile_definitions(topaz PUBLIC -DTZ_VERSION=\"${TZ_VERSION}\" -DTZ_SILENCED=0 -DGLFW_INCLUDE_NONE)

if(${TOPAZ_DEBUG})
	message(STATUS "BuildConfig: Debug")
	configure_debug(topaz)
elseif(${TOPAZ_PROFILE})
	message(STATUS "BuildConfig: Profile")
	configure_profile(topaz)
else()
	message(STATUS "BuildConfig: Release")
	configure_release(topaz)
endif()

add_subdirectory(lib)
add_subdirectory(tools)
include(cmake/shader_header_exporter.cmake)
include(cmake/shader_compiler.cmake)
add_subdirectory(demo)
enable_testing()
add_subdirectory(test)
#
#add_tz_test(
#    NAME default_test
#    SOURCE_FILES test/test.cpp
#)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	# GCC/Clang options
	target_compile_options(topaz PRIVATE -Wall -Wextra -Wdelete-non-virtual-dtor -pedantic-errors)
	if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		# Clang-only options
		# Use GNU's `, __VA_ARGS__` extension (used for tz_assert, tz_error, tz_report)
		target_compile_options(topaz PRIVATE -Wno-gnu-zero-variadic-macro-arguments)
	elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
		# GCC-only options
		target_compile_options(topaz PRIVATE -Wshadow=local)
	endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	# MSVC-only options
endif()
target_include_directories(topaz PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(topaz PUBLIC debugbreak glfw)
