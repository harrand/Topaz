cmake_minimum_required(VERSION 3.9)
include(cmake/util.cmake)
include(cmake/exe_config.cmake)
include(cmake/setup_configs.cmake)
include(cmake/platform.cmake)

configure_file(version.txt version_txt_dummy.xml)
file(READ version.txt TZ_VERSION_TXT)
string(REGEX MATCH "[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+" TZ_VERSION ${TZ_VERSION_TXT})

#file(READ ./version.txt TZ_VERSION)
# Reconfigure on changes to ./version.txt
#configure_file(./version.txt topaz_version.xml)
set(TZ_CXX_STANDARD 20)

project(topaz VERSION ${TZ_VERSION}
	DESCRIPTION "C++20 Graphics Engine")

include(cmake/project.cmake)

find_package(Threads)

add_library(topaz STATIC
	src/tz/tz.cpp
	src/tz/tz.hpp
	src/tz/tz.inl
	# tz::core
	src/tz/core/algorithms/static.hpp
	src/tz/core/data/basic_list.hpp
	src/tz/core/data/contiguous_free_list.cpp
	src/tz/core/data/contiguous_free_list.hpp
	src/tz/core/data/contiguous_free_list.inl
	src/tz/core/data/data_store.cpp
	src/tz/core/data/data_store.hpp
	src/tz/core/data/enum_field.hpp
	src/tz/core/data/enum_field.inl
	src/tz/core/data/grid_view.hpp
	src/tz/core/data/grid_view.inl
	src/tz/core/data/handle.hpp
	src/tz/core/data/maybe_owned_list.hpp
	src/tz/core/data/maybe_owned_list.inl
	src/tz/core/data/polymorphic_list.hpp
	src/tz/core/data/polymorphic_list.inl
	src/tz/core/data/quat.cpp
	src/tz/core/data/quat.hpp
	src/tz/core/data/transform_hierarchy.hpp
	src/tz/core/data/transform_hierarchy.inl
	src/tz/core/data/trs.cpp
	src/tz/core/data/trs.hpp
	src/tz/core/data/vector.cpp
	src/tz/core/data/vector.hpp
	src/tz/core/data/vector.inl
	src/tz/core/data/version.cpp
	src/tz/core/data/version.hpp
	src/tz/core/job/job.cpp
	src/tz/core/job/job.hpp
	src/tz/core/job/api/job.hpp
	src/tz/core/job/impl/concurrentqueue_blocking/job.cpp
	src/tz/core/job/impl/concurrentqueue_blocking/job.hpp
	src/tz/core/job/impl/threadpool_lfq/job.cpp
	src/tz/core/job/impl/threadpool_lfq/job.hpp
	src/tz/core/memory/allocators/adapter.hpp
	src/tz/core/memory/allocators/fallback.hpp
	src/tz/core/memory/allocators/linear.cpp
	src/tz/core/memory/allocators/linear.hpp
	src/tz/core/memory/allocators/malloc.cpp
	src/tz/core/memory/allocators/malloc.hpp
	src/tz/core/memory/allocators/null.hpp
	src/tz/core/memory/allocators/slab.cpp
	src/tz/core/memory/allocators/slab.hpp
	src/tz/core/memory/allocators/stack.hpp
	src/tz/core/memory/allocators/stack.inl
	src/tz/core/memory/clone.hpp
	src/tz/core/memory/maybe_owned_ptr.hpp
	src/tz/core/memory/maybe_owned_ptr.inl
	src/tz/core/memory/memblk.hpp

	src/tz/core/callback.hpp
	src/tz/core/debug.cpp
	src/tz/core/debug.hpp
	src/tz/core/debug.inl
	src/tz/core/endian.cpp
	src/tz/core/endian.hpp
	src/tz/core/engine_info.cpp
	src/tz/core/engine_info.hpp
	src/tz/core/engine_info.inl
	src/tz/core/game_info.hpp
	src/tz/core/imported_text.hpp
	src/tz/core/macros.hpp
	src/tz/core/matrix.hpp
	src/tz/core/matrix.inl
	src/tz/core/matrix_transform.cpp
	src/tz/core/matrix_transform.hpp
	src/tz/core/time.cpp
	src/tz/core/time.hpp
	src/tz/core/types.hpp
	src/tz/core/tz_core.cpp
	src/tz/core/tz_core.hpp
	# tz::wsi
	src/tz/wsi/keyboard.cpp
	src/tz/wsi/keyboard.hpp
	src/tz/wsi/monitor.cpp
	src/tz/wsi/monitor.hpp
	src/tz/wsi/mouse.cpp
	src/tz/wsi/mouse.hpp
	src/tz/wsi/window.cpp
	src/tz/wsi/window.hpp
	src/tz/wsi/wsi.hpp
	src/tz/wsi/wsi.cpp
	# tz/wsi api
	src/tz/wsi/api/keyboard.hpp
	src/tz/wsi/api/mouse.hpp
	src/tz/wsi/api/window.hpp

	# tz/wsi impl (linux)
	src/tz/wsi/impl/linux/keyboard.cpp
	src/tz/wsi/impl/linux/keyboard.hpp
	src/tz/wsi/impl/linux/monitor.cpp
	src/tz/wsi/impl/linux/monitor.hpp
	src/tz/wsi/impl/linux/wsi_linux.cpp
	src/tz/wsi/impl/linux/wsi_linux.hpp
	src/tz/wsi/impl/linux/window.cpp
	src/tz/wsi/impl/linux/window.hpp
	# tz/wsi impl (windows)
	src/tz/wsi/impl/windows/detail/wgl_ext.hpp
	src/tz/wsi/impl/windows/detail/winapi.hpp
	src/tz/wsi/impl/windows/keyboard.cpp
	src/tz/wsi/impl/windows/keyboard.hpp
	src/tz/wsi/impl/windows/keyboard.inl
	src/tz/wsi/impl/windows/monitor.cpp
	src/tz/wsi/impl/windows/monitor.hpp
	src/tz/wsi/impl/windows/wsi_windows.cpp
	src/tz/wsi/impl/windows/wsi_windows.hpp
	src/tz/wsi/impl/windows/window.cpp
	src/tz/wsi/impl/windows/window.hpp
	# tz::dbgui
	src/tz/dbgui/dbgui.cpp
	src/tz/dbgui/dbgui.hpp
	# tz::gl (API)
	src/tz/gl/api/component.hpp
	src/tz/gl/api/device.cpp
	src/tz/gl/api/device.hpp
	src/tz/gl/api/device.inl
	src/tz/gl/api/output.hpp
	src/tz/gl/api/renderer.cpp
	src/tz/gl/api/renderer.hpp
	src/tz/gl/api/resource.hpp
	src/tz/gl/api/schedule.cpp
	src/tz/gl/api/schedule.hpp
	src/tz/gl/api/shader.cpp
	src/tz/gl/api/shader.hpp
	# tz::gl
	src/tz/gl/component.hpp
	src/tz/gl/device.cpp
	src/tz/gl/device.hpp
	src/tz/gl/draw.hpp
	src/tz/gl/output.cpp
	src/tz/gl/output.hpp
	src/tz/gl/renderer.hpp
	src/tz/gl/resource.cpp
	src/tz/gl/resource.hpp
	src/tz/gl/resource.inl
	src/tz/gl/tz_gl.cpp
	src/tz/gl/tz_gl.hpp
	# tz::gl (Common Impl)
	src/tz/gl/impl/common/device.dbgui.cpp
	src/tz/gl/impl/common/device.dbgui.hpp
	src/tz/gl/impl/common/renderer.dbgui.cpp
	src/tz/gl/impl/common/renderer.dbgui.hpp
	# tz::gl (Vulkan Frontend)
	src/tz/gl/impl/vulkan/component.cpp
	src/tz/gl/impl/vulkan/component.hpp
	src/tz/gl/impl/vulkan/convert.hpp
	src/tz/gl/impl/vulkan/convert.inl
	src/tz/gl/impl/vulkan/device2.cpp
	src/tz/gl/impl/vulkan/device2.hpp
	src/tz/gl/impl/vulkan/renderer2.cpp
	src/tz/gl/impl/vulkan/renderer2.hpp
	# tz::gl (OpenGL Frontend)
	src/tz/gl/impl/opengl/component.cpp
	src/tz/gl/impl/opengl/component.hpp
	src/tz/gl/impl/opengl/convert.inl
	src/tz/gl/impl/opengl/convert.hpp
	src/tz/gl/impl/opengl/device.cpp
	src/tz/gl/impl/opengl/device.hpp
	src/tz/gl/impl/opengl/renderer.cpp
	src/tz/gl/impl/opengl/renderer.hpp
	# tz::gl (Vulkan Backend)
	src/tz/gl/impl/vulkan/detail/buffer.cpp
	src/tz/gl/impl/vulkan/detail/buffer.hpp
	src/tz/gl/impl/vulkan/detail/command.cpp
	src/tz/gl/impl/vulkan/detail/command.hpp
	src/tz/gl/impl/vulkan/detail/debugname.hpp
	src/tz/gl/impl/vulkan/detail/debugname.inl
	src/tz/gl/impl/vulkan/detail/descriptors.cpp
	src/tz/gl/impl/vulkan/detail/descriptors.hpp
	src/tz/gl/impl/vulkan/detail/draw.hpp
	src/tz/gl/impl/vulkan/detail/extensions.hpp
	src/tz/gl/impl/vulkan/detail/features.hpp
	src/tz/gl/impl/vulkan/detail/fence.cpp
	src/tz/gl/impl/vulkan/detail/fence.hpp
	src/tz/gl/impl/vulkan/detail/fixed_function.cpp
	src/tz/gl/impl/vulkan/detail/fixed_function.hpp
	src/tz/gl/impl/vulkan/detail/gpu_mem.hpp
	src/tz/gl/impl/vulkan/detail/graphics_pipeline.cpp
	src/tz/gl/impl/vulkan/detail/graphics_pipeline.hpp
	src/tz/gl/impl/vulkan/detail/tz_vulkan.cpp
	src/tz/gl/impl/vulkan/detail/tz_vulkan.hpp
	src/tz/gl/impl/vulkan/detail/image.cpp
	src/tz/gl/impl/vulkan/detail/image.hpp
	src/tz/gl/impl/vulkan/detail/image_view.cpp
	src/tz/gl/impl/vulkan/detail/image_view.hpp
	src/tz/gl/impl/vulkan/detail/image_format.hpp
	src/tz/gl/impl/vulkan/detail/logical_device.cpp
	src/tz/gl/impl/vulkan/detail/logical_device.hpp
	src/tz/gl/impl/vulkan/detail/pipeline_layout.cpp
	src/tz/gl/impl/vulkan/detail/pipeline_layout.hpp
	src/tz/gl/impl/vulkan/detail/sampler.cpp
	src/tz/gl/impl/vulkan/detail/sampler.hpp
	src/tz/gl/impl/vulkan/detail/semaphore.cpp
	src/tz/gl/impl/vulkan/detail/semaphore.hpp
	src/tz/gl/impl/vulkan/detail/shader.cpp
	src/tz/gl/impl/vulkan/detail/shader.hpp
	src/tz/gl/impl/vulkan/detail/swapchain.cpp
	src/tz/gl/impl/vulkan/detail/swapchain.hpp
	src/tz/gl/impl/vulkan/detail/hardware/physical_device.cpp
	src/tz/gl/impl/vulkan/detail/hardware/physical_device.hpp
	src/tz/gl/impl/vulkan/detail/hardware/queue.cpp
	src/tz/gl/impl/vulkan/detail/hardware/queue.hpp
	# tz::gl (OpenGL Backend)
	src/tz/gl/impl/opengl/detail/buffer.cpp
	src/tz/gl/impl/opengl/detail/buffer.hpp
	src/tz/gl/impl/opengl/detail/draw.hpp
	src/tz/gl/impl/opengl/detail/framebuffer.cpp
	src/tz/gl/impl/opengl/detail/framebuffer.hpp
	src/tz/gl/impl/opengl/detail/image.cpp
	src/tz/gl/impl/opengl/detail/image.hpp
	src/tz/gl/impl/opengl/detail/image_format.hpp
	src/tz/gl/impl/opengl/detail/renderbuffer.cpp
	src/tz/gl/impl/opengl/detail/renderbuffer.hpp
	src/tz/gl/impl/opengl/detail/sampler.hpp
	src/tz/gl/impl/opengl/detail/shader.cpp
	src/tz/gl/impl/opengl/detail/shader.hpp
	src/tz/gl/impl/opengl/detail/tz_opengl.cpp
	src/tz/gl/impl/opengl/detail/tz_opengl.hpp
	src/tz/gl/impl/opengl/detail/vertex_array.cpp
	src/tz/gl/impl/opengl/detail/vertex_array.hpp

	# tz::io
	# gltf
	src/tz/io/gltf.cpp
	src/tz/io/gltf.hpp
	# image
	src/tz/io/image.cpp
	src/tz/io/image.hpp
	# ttf
	src/tz/io/ttf.cpp
	src/tz/io/ttf.hpp
	src/tz/io/tz_io.cpp
	src/tz/io/tz_io.hpp

	# tz::lua
	src/tz/lua/api.cpp
	src/tz/lua/api.hpp
	src/tz/lua/lua.hpp
	src/tz/lua/slab.cpp
	src/tz/lua/slab.hpp
	src/tz/lua/state.cpp
	src/tz/lua/state.hpp

	# tz::ren
	src/tz/ren/animation.cpp
	src/tz/ren/animation.dbgui.cpp
	src/tz/ren/animation.hpp
	src/tz/ren/api.hpp
	src/tz/ren/mesh.cpp
	src/tz/ren/mesh.dbgui.cpp
	src/tz/ren/mesh.hpp
	src/tz/ren/text.cpp
	src/tz/ren/text.hpp
	src/tz/ren/tz_ren.cpp
	src/tz/ren/tz_ren.hpp
	)

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
	configure_windows(topaz)
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
	configure_linux(topaz)
else()
	message(FATAL_ERROR "Unrecognised Platform ${CMAKE_HOST_SYSTEM_NAME}")
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	target_compile_definitions(topaz PUBLIC "-DTZ_DEBUG=1")
	tz_configure_debug(TARGET topaz)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
	target_compile_definitions(topaz PUBLIC "-DTZ_DEBUG=0")
	tz_configure_release(TARGET topaz)
endif()

if(${TZ_PROFILE})
	message(STATUS "Topaz - Profiling enabled in this build")
	target_compile_definitions(topaz PUBLIC "-DTZ_PROFILE=1")
	tz_configure_profile(TARGET topaz)
else()
	message(STATUS "Topaz - Profiling disabled in this build")
	target_compile_definitions(topaz PUBLIC "-DTZ_PROFILE=0")
endif()

message(STATUS "Topaz v${TZ_VERSION}")
set_target_properties(topaz PROPERTIES
	CXX_STANDARD ${TZ_CXX_STANDARD}
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF)

if(${TOPAZ_RENDER_API} MATCHES "Vulkan")
	configure_vulkan(topaz)
elseif(${TOPAZ_RENDER_API} MATCHES "OpenGL")
	configure_opengl(topaz)
else()
	if(${TOPAZ_RENDER_API})
		message(SEND_ERROR "Render API \"${TOPAZ_BUILDCONFIG}\" not recognised.")
	else()
		message(SEND_ERROR "Render API not specified.")
	endif()
	message(FATAL_ERROR "Invalid render API. Values can be \"Vulkan\" or \"OpenGL\"")
endif()

configure_common(topaz)

message(STATUS "Render API: ${TOPAZ_RENDER_API}")

add_subdirectory(lib)

target_include_directories(topaz PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(topaz PUBLIC debugbreak imgui imgui_memory_editor glad textc_lib concurrentqueue stb_image)
target_link_libraries(topaz PRIVATE Threads::Threads tz_lua msdfgen-core nlohmann_json)

add_subdirectory(tools)
include(cmake/shader_compiler.cmake)
add_shader(
	TARGET topaz
	SHADERS
		src/tz/dbgui/dbgui.vertex.tzsl
		src/tz/dbgui/dbgui.fragment.tzsl
		src/tz/dbgui/empty.vertex.tzsl
		src/tz/dbgui/empty.fragment.tzsl

		src/tz/ren/shaders/animation.vertex.tzsl
		src/tz/ren/shaders/animation.fragment.tzsl
		src/tz/ren/shaders/mesh.compute.tzsl
		src/tz/ren/shaders/mesh.vertex.tzsl
		src/tz/ren/shaders/mesh.fragment.tzsl
		src/tz/ren/shaders/text.vertex.tzsl
		src/tz/ren/shaders/text.fragment.tzsl
		src/tz/ren/shaders/text.compute.tzsl
	)

add_text(
	TARGET topaz
	INPUT_DIR ${PROJECT_SOURCE_DIR}
	OUTPUT_DIR ${PROJECT_BINARY_DIR}
	TEXT_FILES
		src/tz/core/tz_core.lua
		src/tz/gl/tz_gl.lua
		src/tz/io/tz_io.lua
		src/tz/ren/tz_ren.lua
		src/tz/wsi/tz_wsi.lua
)

add_subdirectory(demo)
include(cmake/test.cmake)
enable_testing()
add_subdirectory(test)

function(add_app)
	cmake_parse_arguments(
		ADD_APP
		""
		"TARGET;CUSTOM_ICON"
		"SOURCE_FILES;SHADER_SOURCES"
		${ARGN}
	)

	add_demo(TARGET ${ADD_APP_TARGET}
		CUSTOM_ICON ${ADD_APP_CUSTOM_ICON}
		SOURCE_FILES ${ADD_APP_SOURCE_FILES}
		SHADER_SOURCES ${ADD_APP_SHADER_SOURCES}
	)
endfunction()
