cmake_minimum_required(VERSION 3.9)
include(cmake/setup_configs.cmake)
include(cmake/shader_compiler.cmake)

file(READ ./version.txt TZ_VERSION)
# Reconfigure on changes to ./version.txt
configure_file(./version.txt topaz_version.xml)
set(CMAKE_CXX_STANDARD 20)

project(TZVK VERSION ${TZ_VERSION})

add_library(TZVK
    src/core/containers/basic_list.hpp
    src/core/containers/polymorphic_list.hpp
    src/core/containers/polymorphic_list.inl

    src/core/assert.hpp
    src/core/engine_info.cpp
    src/core/engine_info.hpp
    src/core/engine_info.inl
    src/core/game_info.hpp
    src/core/report.hpp
    src/core/tz.cpp
    src/core/tz.hpp
    src/core/tz.inl
    src/core/window_functionality.cpp
    src/core/window_functionality.hpp
    src/core/window.cpp
    src/core/window.hpp

    src/gl/vk/impl/hardware/device_filter.cpp
    src/gl/vk/impl/hardware/device_filter.hpp
    src/gl/vk/impl/hardware/device_filter.inl
    src/gl/vk/impl/hardware/device.cpp
    src/gl/vk/impl/hardware/device.hpp
    src/gl/vk/impl/hardware/queue_family.cpp
    src/gl/vk/impl/hardware/queue_family.hpp
    src/gl/vk/impl/hardware/swapchain_selector.cpp
    src/gl/vk/impl/hardware/swapchain_selector.hpp
    src/gl/vk/impl/hardware/swapchain_support.hpp

    src/gl/vk/impl/pipeline/colour_blend_state.cpp
    src/gl/vk/impl/pipeline/colour_blend_state.hpp
    src/gl/vk/impl/pipeline/framebuffer_attachments.cpp
    src/gl/vk/impl/pipeline/framebuffer_attachments.hpp
    src/gl/vk/impl/pipeline/graphics_pipeline.cpp
    src/gl/vk/impl/pipeline/graphics_pipeline.hpp
    src/gl/vk/impl/pipeline/input_assembly.cpp
    src/gl/vk/impl/pipeline/input_assembly.hpp
    src/gl/vk/impl/pipeline/layout.cpp
    src/gl/vk/impl/pipeline/layout.hpp
    src/gl/vk/impl/pipeline/multisample_state.cpp
    src/gl/vk/impl/pipeline/multisample_state.hpp
    src/gl/vk/impl/pipeline/rasteriser_state.cpp
    src/gl/vk/impl/pipeline/rasteriser_state.hpp
    src/gl/vk/impl/pipeline/render_pass.cpp
    src/gl/vk/impl/pipeline/render_pass.hpp
    src/gl/vk/impl/pipeline/shader_compiler.cpp
    src/gl/vk/impl/pipeline/shader_compiler.hpp
    src/gl/vk/impl/pipeline/shader_module.cpp
    src/gl/vk/impl/pipeline/shader_module.hpp
    src/gl/vk/impl/pipeline/shader_stage.cpp
    src/gl/vk/impl/pipeline/shader_stage.hpp
    src/gl/vk/impl/pipeline/vertex_input.cpp
    src/gl/vk/impl/pipeline/vertex_input.hpp
    src/gl/vk/impl/pipeline/viewport.cpp
    src/gl/vk/impl/pipeline/viewport.hpp

    src/gl/vk/impl/setup/extension_list.cpp
    src/gl/vk/impl/setup/extension_list.hpp
    src/gl/vk/impl/setup/image_view.cpp
    src/gl/vk/impl/setup/image_view.hpp
    src/gl/vk/impl/setup/logical_device.cpp
    src/gl/vk/impl/setup/logical_device.hpp
    src/gl/vk/impl/setup/swapchain.cpp
    src/gl/vk/impl/setup/swapchain.hpp
    src/gl/vk/impl/setup/vulkan_instance.cpp
    src/gl/vk/impl/setup/vulkan_instance.hpp
    src/gl/vk/impl/setup/window_surface.cpp
    src/gl/vk/impl/setup/window_surface.hpp

    src/gl/vk/impl/validation/debug_callback.hpp
    src/gl/vk/impl/validation/debug_messenger.cpp
    src/gl/vk/impl/validation/debug_messenger.hpp
    src/gl/vk/impl/validation/debug_messenger.inl
    src/gl/vk/impl/validation/layer.cpp
    src/gl/vk/impl/validation/layer.hpp

    src/gl/vk/attachment.cpp
    src/gl/vk/attachment.hpp
    src/gl/vk/framebuffer.cpp
    src/gl/vk/framebuffer.hpp
    src/gl/vk/image.hpp
    src/gl/vk/render_pass.cpp
    src/gl/vk/render_pass.hpp
    src/gl/vk/render_pass.inl
    src/gl/vk/tz_vulkan.cpp
    src/gl/vk/tz_vulkan.hpp
    )


message(STATUS "Topaz v${TZ_VERSION}")

if(${TOPAZ_OGL})
    # Vulkan + OGL
    message(STATUS "RenderAPI: OpenGL (\"OGL\")")
    configure_opengl(TZVK)
elseif(${TOPAZ_VULKAN})
    # Vulkan
    message(STATUS "RenderAPI: Vulkan (\"VK\")")
    configure_vulkan(TZVK)
endif()
target_compile_definitions(TZVK PUBLIC -DTZ_VERSION=\"${TZ_VERSION}\" -DTZ_SILENCED=0)

if(${TOPAZ_DEBUG})
    message(STATUS "BuildConfig: Debug")
    configure_debug(TZVK)
else()
    message(STATUS "BuildConfig: Release")
    configure_release(TZVK)
endif()

add_subdirectory(lib)
add_subdirectory(demo)

target_include_directories(TZVK PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(TZVK PUBLIC debugbreak glfw)