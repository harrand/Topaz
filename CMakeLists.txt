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

add_subdirectory(lib/HDK)
include(cmake/project.cmake)

hdk_add_library(TARGET topaz
	SOURCES
	# tz::core
	src/tz/core/containers/basic_list.hpp
	src/tz/core/containers/enum_field.hpp
	src/tz/core/containers/enum_field.inl
	src/tz/core/containers/grid_view.hpp
	src/tz/core/containers/grid_view.inl
	src/tz/core/containers/polymorphic_list.hpp
	src/tz/core/containers/polymorphic_list.inl
	src/tz/core/algorithms/static.hpp
	src/tz/core/callback.hpp
	src/tz/core/engine_info.cpp
	src/tz/core/engine_info.hpp
	src/tz/core/engine_info.inl
	src/tz/core/game_info.hpp
	src/tz/core/imported_text.hpp
	src/tz/core/matrix_transform.cpp
	src/tz/core/matrix_transform.hpp
	src/tz/core/matrix.hpp
	src/tz/core/matrix.inl
	src/tz/core/memory.hpp
	src/tz/core/memory.inl
	src/tz/core/time.cpp
	src/tz/core/time.hpp
	src/tz/core/tz.cpp
	src/tz/core/tz.hpp
	src/tz/core/tz.inl
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
	src/tz/gl/api/device.hpp
	src/tz/gl/api/output.hpp
	src/tz/gl/api/renderer.hpp
	src/tz/gl/api/resource.hpp
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
	src/tz/gl/tz_gl.hpp
	# tz::gl (Common Impl)
	src/tz/gl/impl/common/device.hpp
	src/tz/gl/impl/common/renderer.cpp
	src/tz/gl/impl/common/renderer.hpp
	src/tz/gl/impl/common/shader.cpp
	src/tz/gl/impl/common/shader.hpp
	# tz::gl (Vulkan Frontend)
	src/tz/gl/impl/vulkan/component.cpp
	src/tz/gl/impl/vulkan/component.hpp
	src/tz/gl/impl/vulkan/convert.hpp
	src/tz/gl/impl/vulkan/convert.inl
	src/tz/gl/impl/vulkan/device.cpp
	src/tz/gl/impl/vulkan/device.hpp
	src/tz/gl/impl/vulkan/renderer.cpp
	src/tz/gl/impl/vulkan/renderer.hpp
	src/tz/gl/impl/vulkan/renderer.inl
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
	src/tz/gl/impl/vulkan/detail/extensions.hpp
	src/tz/gl/impl/vulkan/detail/features.hpp
	src/tz/gl/impl/vulkan/detail/fence.cpp
	src/tz/gl/impl/vulkan/detail/fence.hpp
	src/tz/gl/impl/vulkan/detail/fixed_function.cpp
	src/tz/gl/impl/vulkan/detail/fixed_function.hpp
	src/tz/gl/impl/vulkan/detail/framebuffer.cpp
	src/tz/gl/impl/vulkan/detail/framebuffer.hpp
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
	src/tz/gl/impl/vulkan/detail/render_pass.cpp
	src/tz/gl/impl/vulkan/detail/render_pass.hpp
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
	)

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
	configure_windows(topaz)
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
	configure_linux(topaz)
else()
	message(FATAL_ERROR "Unrecognised Platform ${CMAKE_HOST_SYSTEM_NAME}")
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
target_link_libraries(topaz PUBLIC imgui glad)

add_subdirectory(tools)
include(cmake/shader_compiler.cmake)
add_shader(
	TARGET topaz
	SHADERS
		src/tz/dbgui/dbgui.vertex.tzsl
		src/tz/dbgui/dbgui.fragment.tzsl
		src/tz/dbgui/empty.vertex.tzsl
		src/tz/dbgui/empty.fragment.tzsl
	)

add_subdirectory(demo)
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
