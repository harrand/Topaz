cmake_minimum_required(VERSION 3.9)
include(cmake/util.cmake)
include(cmake/exe_config.cmake)
include(cmake/setup_configs.cmake)

file(READ ./version.txt TZ_VERSION)
# Reconfigure on changes to ./version.txt
configure_file(./version.txt topaz_version.xml)
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
	src/tz/core/peripherals/keyboard.cpp
	src/tz/core/peripherals/keyboard.hpp
	src/tz/core/peripherals/monitor.cpp
	src/tz/core/peripherals/monitor.hpp
	src/tz/core/peripherals/mouse.cpp
	src/tz/core/peripherals/mouse.hpp
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
	src/tz/core/vector.hpp
	src/tz/core/vector.inl
	src/tz/core/version.cpp
	src/tz/core/version.hpp
	src/tz/core/version.inl
	src/tz/core/window_functionality.cpp
	src/tz/core/window_functionality.hpp
	src/tz/core/window_functionality.inl
	src/tz/core/window.cpp
	src/tz/core/window.hpp
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
	src/tz/gl/impl/frontend/common/device.hpp
	src/tz/gl/impl/frontend/common/renderer.cpp
	src/tz/gl/impl/frontend/common/renderer.hpp
	src/tz/gl/impl/frontend/common/shader.cpp
	src/tz/gl/impl/frontend/common/shader.hpp
	# tz::gl (Vulkan Frontend)
	src/tz/gl/impl/frontend/vk2/component.cpp
	src/tz/gl/impl/frontend/vk2/component.hpp
	src/tz/gl/impl/frontend/vk2/convert.hpp
	src/tz/gl/impl/frontend/vk2/convert.inl
	src/tz/gl/impl/frontend/vk2/device.cpp
	src/tz/gl/impl/frontend/vk2/device.hpp
	src/tz/gl/impl/frontend/vk2/renderer.cpp
	src/tz/gl/impl/frontend/vk2/renderer.hpp
	src/tz/gl/impl/frontend/vk2/renderer.inl
	# tz::gl (OpenGL Frontend)
	src/tz/gl/impl/frontend/ogl2/component.cpp
	src/tz/gl/impl/frontend/ogl2/component.hpp
	src/tz/gl/impl/frontend/ogl2/convert.inl
	src/tz/gl/impl/frontend/ogl2/convert.hpp
	src/tz/gl/impl/frontend/ogl2/device.cpp
	src/tz/gl/impl/frontend/ogl2/device.hpp
	src/tz/gl/impl/frontend/ogl2/renderer.cpp
	src/tz/gl/impl/frontend/ogl2/renderer.hpp
	# tz::gl (Vulkan Backend)
	src/tz/gl/impl/backend/vk2/buffer.cpp
	src/tz/gl/impl/backend/vk2/buffer.hpp
	src/tz/gl/impl/backend/vk2/command.cpp
	src/tz/gl/impl/backend/vk2/command.hpp
	src/tz/gl/impl/backend/vk2/debugname.hpp
	src/tz/gl/impl/backend/vk2/debugname.inl
	src/tz/gl/impl/backend/vk2/descriptors.cpp
	src/tz/gl/impl/backend/vk2/descriptors.hpp
	src/tz/gl/impl/backend/vk2/extensions.hpp
	src/tz/gl/impl/backend/vk2/features.hpp
	src/tz/gl/impl/backend/vk2/fence.cpp
	src/tz/gl/impl/backend/vk2/fence.hpp
	src/tz/gl/impl/backend/vk2/fixed_function.cpp
	src/tz/gl/impl/backend/vk2/fixed_function.hpp
	src/tz/gl/impl/backend/vk2/framebuffer.cpp
	src/tz/gl/impl/backend/vk2/framebuffer.hpp
	src/tz/gl/impl/backend/vk2/gpu_mem.hpp
	src/tz/gl/impl/backend/vk2/graphics_pipeline.cpp
	src/tz/gl/impl/backend/vk2/graphics_pipeline.hpp
	src/tz/gl/impl/backend/vk2/tz_vulkan.cpp
	src/tz/gl/impl/backend/vk2/tz_vulkan.hpp
	src/tz/gl/impl/backend/vk2/image.cpp
	src/tz/gl/impl/backend/vk2/image.hpp
	src/tz/gl/impl/backend/vk2/image_view.cpp
	src/tz/gl/impl/backend/vk2/image_view.hpp
	src/tz/gl/impl/backend/vk2/image_format.hpp
	src/tz/gl/impl/backend/vk2/logical_device.cpp
	src/tz/gl/impl/backend/vk2/logical_device.hpp
	src/tz/gl/impl/backend/vk2/pipeline_layout.cpp
	src/tz/gl/impl/backend/vk2/pipeline_layout.hpp
	src/tz/gl/impl/backend/vk2/render_pass.cpp
	src/tz/gl/impl/backend/vk2/render_pass.hpp
	src/tz/gl/impl/backend/vk2/sampler.cpp
	src/tz/gl/impl/backend/vk2/sampler.hpp
	src/tz/gl/impl/backend/vk2/semaphore.cpp
	src/tz/gl/impl/backend/vk2/semaphore.hpp
	src/tz/gl/impl/backend/vk2/shader.cpp
	src/tz/gl/impl/backend/vk2/shader.hpp
	src/tz/gl/impl/backend/vk2/swapchain.cpp
	src/tz/gl/impl/backend/vk2/swapchain.hpp
	src/tz/gl/impl/backend/vk2/hardware/physical_device.cpp
	src/tz/gl/impl/backend/vk2/hardware/physical_device.hpp
	src/tz/gl/impl/backend/vk2/hardware/queue.cpp
	src/tz/gl/impl/backend/vk2/hardware/queue.hpp
	# tz::gl (OpenGL Backend)
	src/tz/gl/impl/backend/ogl2/buffer.cpp
	src/tz/gl/impl/backend/ogl2/buffer.hpp
	src/tz/gl/impl/backend/ogl2/draw.hpp
	src/tz/gl/impl/backend/ogl2/framebuffer.cpp
	src/tz/gl/impl/backend/ogl2/framebuffer.hpp
	src/tz/gl/impl/backend/ogl2/image.cpp
	src/tz/gl/impl/backend/ogl2/image.hpp
	src/tz/gl/impl/backend/ogl2/image_format.hpp
	src/tz/gl/impl/backend/ogl2/renderbuffer.cpp
	src/tz/gl/impl/backend/ogl2/renderbuffer.hpp
	src/tz/gl/impl/backend/ogl2/sampler.hpp
	src/tz/gl/impl/backend/ogl2/shader.cpp
	src/tz/gl/impl/backend/ogl2/shader.hpp
	src/tz/gl/impl/backend/ogl2/tz_opengl.cpp
	src/tz/gl/impl/backend/ogl2/tz_opengl.hpp
	src/tz/gl/impl/backend/ogl2/vertex_array.cpp
	src/tz/gl/impl/backend/ogl2/vertex_array.hpp
	)

message(STATUS "Topaz v${TZ_VERSION}")
set_target_properties(topaz PROPERTIES
	CXX_STANDARD ${TZ_CXX_STANDARD}
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF)
# New Cmake presets code:
if(${TOPAZ_RENDER_API} MATCHES "Vulkan")
	set(TOPAZ_RENDERAPI "vulkan")
elseif(${TOPAZ_RENDER_API} MATCHES "OpenGL")
	set(TOPAZ_RENDERAPI "opengl")
endif()

if(${TOPAZ_RENDERAPI} MATCHES "vulkan")
	configure_vulkan(topaz)
elseif(${TOPAZ_RENDERAPI} MATCHES "opengl")
	configure_opengl(topaz)
else()
	if(${TOPAZ_RENDERAPI})
		message(SEND_ERROR "Render API \"${TOPAZ_BUILDCONFIG}\" not recognised.")
	else()
		message(SEND_ERROR "Render API not specified.")
	endif()
	message(FATAL_ERROR "Invalid render API. Values can be \"vulkan\" or \"opengl\"")
endif()

configure_common(topaz)

message(STATUS "Render API: ${TOPAZ_RENDERAPI}")

add_subdirectory(lib)

target_include_directories(topaz PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(topaz PUBLIC glfw imgui)

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
