cmake_minimum_required(VERSION 3.9)
include(cmake/setup_configs.cmake)
include(cmake/shader_compiler.cmake)

file(READ ./version.txt TZ_VERSION)
# Reconfigure on changes to ./version.txt
configure_file(./version.txt topaz_version.xml)
set(CXX_STANDARD_REQUIRED 20)
set(CMAKE_CXX_STANDARD 20)

project(topaz VERSION ${TZ_VERSION})

add_library(topaz
    # tz::core
    src/core/containers/basic_list.hpp
    src/core/containers/enum_field.hpp
    src/core/containers/enum_field.inl
    src/core/containers/polymorphic_list.hpp
    src/core/containers/polymorphic_list.inl

    src/core/algorithms/static.hpp

    src/core/assert.hpp
    src/core/engine_info.cpp
    src/core/engine_info.hpp
    src/core/engine_info.inl
    src/core/game_info.hpp
    src/core/matrix_transform.cpp
    src/core/matrix_transform.hpp
    src/core/matrix.hpp
    src/core/matrix.inl
    src/core/report.hpp
    src/core/tz.cpp
    src/core/tz.hpp
    src/core/tz.inl
    src/core/vector.hpp
    src/core/vector.inl
    src/core/window_functionality.cpp
    src/core/window_functionality.hpp
    src/core/window_functionality.inl
    src/core/window.cpp
    src/core/window.hpp

    # tz::gl
    src/gl/device.hpp
    src/gl/mesh.cpp
    src/gl/mesh.hpp
    src/gl/render_pass.hpp
    src/gl/renderer.hpp
    src/gl/shader.hpp

    # tz::gl (API)
    src/gl/api/device.hpp
    src/gl/api/render_pass.hpp
    src/gl/api/renderer.hpp
    src/gl/api/shader.hpp

    # tz::gl (Vulkan Frontend)
    src/gl/impl/vk/device.cpp
    src/gl/impl/vk/device.hpp
    src/gl/impl/vk/render_pass.cpp
    src/gl/impl/vk/render_pass.hpp
    src/gl/impl/vk/renderer.cpp
    src/gl/impl/vk/renderer.hpp
    src/gl/impl/vk/shader.cpp
    src/gl/impl/vk/shader.hpp

    # tz::gl (OpenGL Frontend)
    src/gl/impl/ogl/device.cpp
    src/gl/impl/ogl/device.hpp
    src/gl/impl/ogl/render_pass.cpp
    src/gl/impl/ogl/render_pass.hpp
    src/gl/impl/ogl/renderer.hpp
    src/gl/impl/ogl/renderer.cpp
    src/gl/impl/ogl/shader.cpp
    src/gl/impl/ogl/shader.hpp

    # tz::gl (Vulkan Backend)
    src/gl/vk/hardware/device_filter.cpp
    src/gl/vk/hardware/device_filter.hpp
    src/gl/vk/hardware/device_filter.inl
    src/gl/vk/hardware/device.cpp
    src/gl/vk/hardware/device.hpp
    src/gl/vk/hardware/memory.cpp
    src/gl/vk/hardware/memory.hpp
    src/gl/vk/hardware/queue_family.cpp
    src/gl/vk/hardware/queue_family.hpp
    src/gl/vk/hardware/swapchain_selector.cpp
    src/gl/vk/hardware/swapchain_selector.hpp
    src/gl/vk/hardware/swapchain_support.hpp

    src/gl/vk/pipeline/colour_blend_state.cpp
    src/gl/vk/pipeline/colour_blend_state.hpp
    src/gl/vk/pipeline/dynamic_state.cpp
    src/gl/vk/pipeline/dynamic_state.hpp
    src/gl/vk/pipeline/graphics_pipeline.cpp
    src/gl/vk/pipeline/graphics_pipeline.hpp
    src/gl/vk/pipeline/input_assembly.cpp
    src/gl/vk/pipeline/input_assembly.hpp
    src/gl/vk/pipeline/layout.cpp
    src/gl/vk/pipeline/layout.hpp
    src/gl/vk/pipeline/multisample_state.cpp
    src/gl/vk/pipeline/multisample_state.hpp
    src/gl/vk/pipeline/rasteriser_state.cpp
    src/gl/vk/pipeline/rasteriser_state.hpp
    src/gl/vk/pipeline/shader_compiler.cpp
    src/gl/vk/pipeline/shader_compiler.hpp
    src/gl/vk/pipeline/shader_module.cpp
    src/gl/vk/pipeline/shader_module.hpp
    src/gl/vk/pipeline/shader_stage.cpp
    src/gl/vk/pipeline/shader_stage.hpp
    src/gl/vk/pipeline/vertex_input.cpp
    src/gl/vk/pipeline/vertex_input.hpp
    src/gl/vk/pipeline/viewport.cpp
    src/gl/vk/pipeline/viewport.hpp

    src/gl/vk/setup/extension_list.cpp
    src/gl/vk/setup/extension_list.hpp
    src/gl/vk/setup/vulkan_instance.cpp
    src/gl/vk/setup/vulkan_instance.hpp
    src/gl/vk/setup/window_surface.cpp
    src/gl/vk/setup/window_surface.hpp

    src/gl/vk/validation/debug_callback.hpp
    src/gl/vk/validation/debug_messenger.cpp
    src/gl/vk/validation/debug_messenger.hpp
    src/gl/vk/validation/debug_messenger.inl
    src/gl/vk/validation/layer.cpp
    src/gl/vk/validation/layer.hpp

    src/gl/vk/hardware/queue.cpp
    src/gl/vk/hardware/queue.hpp

    src/gl/vk/attachment.cpp
    src/gl/vk/attachment.hpp
    src/gl/vk/attribute_binding_description.cpp
    src/gl/vk/attribute_binding_description.hpp
    src/gl/vk/buffer.cpp
    src/gl/vk/buffer.hpp
    src/gl/vk/command.cpp
    src/gl/vk/command.hpp
    src/gl/vk/command.inl
    src/gl/vk/descriptor_set_layout.cpp
    src/gl/vk/descriptor_set_layout.hpp
    src/gl/vk/descriptor.cpp
    src/gl/vk/descriptor.hpp
    src/gl/vk/descriptor.inl
    src/gl/vk/fence.cpp
    src/gl/vk/fence.hpp
    src/gl/vk/frame_admin.cpp
    src/gl/vk/frame_admin.hpp
    src/gl/vk/frame_admin.inl
    src/gl/vk/framebuffer.cpp
    src/gl/vk/framebuffer.hpp
    src/gl/vk/image_view.cpp
    src/gl/vk/image_view.hpp
    src/gl/vk/image.hpp
    src/gl/vk/image.cpp
    src/gl/vk/logical_device.cpp
    src/gl/vk/logical_device.hpp
    src/gl/vk/present.cpp
    src/gl/vk/present.hpp
    src/gl/vk/render_pass.cpp
    src/gl/vk/render_pass.hpp
    src/gl/vk/render_pass.inl
    src/gl/vk/sampler.cpp
    src/gl/vk/sampler.hpp
    src/gl/vk/semaphore.cpp
    src/gl/vk/semaphore.hpp
    src/gl/vk/submit.cpp
    src/gl/vk/submit.hpp
    src/gl/vk/swapchain.cpp
    src/gl/vk/swapchain.hpp
    src/gl/vk/tz_vulkan.cpp
    src/gl/vk/tz_vulkan.hpp
    src/gl/vk/vertex_binding_description.cpp
    src/gl/vk/vertex_binding_description.hpp

    # tz::gl (OpenGL Backend)
    src/gl/ogl/setup/opengl_instance.cpp
    src/gl/ogl/setup/opengl_instance.hpp
    src/gl/ogl/tz_opengl.hpp
    src/gl/ogl/tz_opengl.cpp
    )


message(STATUS "Topaz v${TZ_VERSION}")

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
else()
    message(STATUS "BuildConfig: Release")
    configure_release(topaz)
endif()

add_subdirectory(lib)
add_subdirectory(demo)
enable_testing()
add_subdirectory(test)

#
#add_tz_test(
#    NAME default_test
#    SOURCE_FILES test/test.cpp
#)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	# For msvc
else()
    # GCC/Clang
    target_compile_options(topaz PRIVATE -Wall -Wextra -Wshadow=local -Wdelete-non-virtual-dtor -pedantic-errors)
endif()
target_include_directories(topaz PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(topaz PUBLIC debugbreak glfw)