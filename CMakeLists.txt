cmake_minimum_required(VERSION 3.9)
include(cmake/setup_configs.cmake)

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
    src/core/interfaces/cloneable.hpp

    src/core/profiling/zone.hpp

    src/core/assert.hpp
    src/core/engine_info.cpp
    src/core/engine_info.hpp
    src/core/engine_info.inl
    src/core/game_info.hpp
    src/core/handle.hpp
    src/core/keyboard_state.cpp
    src/core/keyboard_state.hpp
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
    src/core/version.cpp
    src/core/version.hpp
    src/core/version.inl
    src/core/window_functionality.cpp
    src/core/window_functionality.hpp
    src/core/window_functionality.inl
    src/core/window.cpp
    src/core/window.hpp

    # tz::gl
    src/gl/buffer.hpp
    src/gl/buffer.inl
    src/gl/component.hpp
    src/gl/device.hpp
    src/gl/input.cpp
    src/gl/input.hpp
    src/gl/mesh.cpp
    src/gl/mesh.hpp
    src/gl/output.cpp
    src/gl/output.hpp
    src/gl/processor.hpp
    src/gl/renderer.hpp
    src/gl/resource.cpp
    src/gl/resource.hpp
    src/gl/shader.hpp
    src/gl/texture.cpp
    src/gl/texture.hpp

    # tz::gl (API)
    src/gl/api/component.hpp
    src/gl/api/device.hpp
    src/gl/api/processor.hpp
    src/gl/api/renderer.hpp
    src/gl/api/resource.hpp
    src/gl/api/shader.hpp

    # tz::gl (Declare)
    src/gl/declare/device.hpp
    src/gl/declare/renderer.hpp
    src/gl/declare/resource.hpp
    src/gl/declare/texture.hpp

    # tz::gl (Common Impl)

    src/gl/impl/frontend/common/component.cpp
    src/gl/impl/frontend/common/component.hpp
    src/gl/impl/frontend/common/component.inl
    src/gl/impl/frontend/common/processor.cpp
    src/gl/impl/frontend/common/processor.hpp
    src/gl/impl/frontend/common/render_pass_info.cpp
    src/gl/impl/frontend/common/render_pass_info.hpp
    src/gl/impl/frontend/common/renderer.cpp
    src/gl/impl/frontend/common/renderer.hpp
    src/gl/impl/frontend/common/shader.cpp
    src/gl/impl/frontend/common/shader.hpp

    # tz::gl (Vulkan Frontend)
    src/gl/impl/frontend/vk/component.cpp
    src/gl/impl/frontend/vk/component.hpp
    src/gl/impl/frontend/vk/convert.hpp
    src/gl/impl/frontend/vk/convert.inl
    src/gl/impl/frontend/vk/device.cpp
    src/gl/impl/frontend/vk/device.hpp
    src/gl/impl/frontend/vk/processor.cpp
    src/gl/impl/frontend/vk/processor.hpp
    src/gl/impl/frontend/vk/render_pass.cpp
    src/gl/impl/frontend/vk/render_pass.hpp
    src/gl/impl/frontend/vk/renderer.cpp
    src/gl/impl/frontend/vk/renderer.hpp
    src/gl/impl/frontend/vk/shader.cpp
    src/gl/impl/frontend/vk/shader.hpp

    # tz::gl (OpenGL Frontend)
    src/gl/impl/frontend/ogl/component.cpp
    src/gl/impl/frontend/ogl/component.hpp
    src/gl/impl/frontend/ogl/convert.hpp
    src/gl/impl/frontend/ogl/convert.inl
    src/gl/impl/frontend/ogl/device.cpp
    src/gl/impl/frontend/ogl/device.hpp
    src/gl/impl/frontend/ogl/processor.cpp
    src/gl/impl/frontend/ogl/processor.hpp
    src/gl/impl/frontend/ogl/renderer.hpp
    src/gl/impl/frontend/ogl/renderer.cpp
    src/gl/impl/frontend/ogl/shader.cpp
    src/gl/impl/frontend/ogl/shader.hpp

    # tz::gl (Vulkan Backend)
    src/gl/impl/backend/vk/hardware/device_filter.cpp
    src/gl/impl/backend/vk/hardware/device_filter.hpp
    src/gl/impl/backend/vk/hardware/device_filter.inl
    src/gl/impl/backend/vk/hardware/device.cpp
    src/gl/impl/backend/vk/hardware/device.hpp
    src/gl/impl/backend/vk/hardware/memory.cpp
    src/gl/impl/backend/vk/hardware/memory.hpp
    src/gl/impl/backend/vk/hardware/queue_family.cpp
    src/gl/impl/backend/vk/hardware/queue_family.hpp
    src/gl/impl/backend/vk/hardware/swapchain_selector.cpp
    src/gl/impl/backend/vk/hardware/swapchain_selector.hpp
    src/gl/impl/backend/vk/hardware/swapchain_support.hpp

    src/gl/impl/backend/vk/pipeline/colour_blend_state.cpp
    src/gl/impl/backend/vk/pipeline/colour_blend_state.hpp
    src/gl/impl/backend/vk/pipeline/compute_pipeline.cpp
    src/gl/impl/backend/vk/pipeline/compute_pipeline.hpp
    src/gl/impl/backend/vk/pipeline/dynamic_state.cpp
    src/gl/impl/backend/vk/pipeline/dynamic_state.hpp
    src/gl/impl/backend/vk/pipeline/graphics_pipeline.cpp
    src/gl/impl/backend/vk/pipeline/graphics_pipeline.hpp
    src/gl/impl/backend/vk/pipeline/input_assembly.cpp
    src/gl/impl/backend/vk/pipeline/input_assembly.hpp
    src/gl/impl/backend/vk/pipeline/layout.cpp
    src/gl/impl/backend/vk/pipeline/layout.hpp
    src/gl/impl/backend/vk/pipeline/multisample_state.cpp
    src/gl/impl/backend/vk/pipeline/multisample_state.hpp
    src/gl/impl/backend/vk/pipeline/rasteriser_state.cpp
    src/gl/impl/backend/vk/pipeline/rasteriser_state.hpp
    src/gl/impl/backend/vk/pipeline/shader_compiler.cpp
    src/gl/impl/backend/vk/pipeline/shader_compiler.hpp
    src/gl/impl/backend/vk/pipeline/shader_module.cpp
    src/gl/impl/backend/vk/pipeline/shader_module.hpp
    src/gl/impl/backend/vk/pipeline/shader_stage.cpp
    src/gl/impl/backend/vk/pipeline/shader_stage.hpp
    src/gl/impl/backend/vk/pipeline/vertex_input.cpp
    src/gl/impl/backend/vk/pipeline/vertex_input.hpp
    src/gl/impl/backend/vk/pipeline/viewport.cpp
    src/gl/impl/backend/vk/pipeline/viewport.hpp

    src/gl/impl/backend/vk/setup/extension_list.cpp
    src/gl/impl/backend/vk/setup/extension_list.hpp
    src/gl/impl/backend/vk/setup/vulkan_instance.cpp
    src/gl/impl/backend/vk/setup/vulkan_instance.hpp
    src/gl/impl/backend/vk/setup/window_surface.cpp
    src/gl/impl/backend/vk/setup/window_surface.hpp

    src/gl/impl/backend/vk/validation/debug_callback.hpp
    src/gl/impl/backend/vk/validation/debug_messenger.cpp
    src/gl/impl/backend/vk/validation/debug_messenger.hpp
    src/gl/impl/backend/vk/validation/debug_messenger.inl
    src/gl/impl/backend/vk/validation/layer.cpp
    src/gl/impl/backend/vk/validation/layer.hpp

    src/gl/impl/backend/vk/hardware/queue.cpp
    src/gl/impl/backend/vk/hardware/queue.hpp

    src/gl/impl/backend/vk/attachment.cpp
    src/gl/impl/backend/vk/attachment.hpp
    src/gl/impl/backend/vk/attribute_binding_description.cpp
    src/gl/impl/backend/vk/attribute_binding_description.hpp
    src/gl/impl/backend/vk/buffer.cpp
    src/gl/impl/backend/vk/buffer.hpp
    src/gl/impl/backend/vk/command.cpp
    src/gl/impl/backend/vk/command.hpp
    src/gl/impl/backend/vk/command.inl
    src/gl/impl/backend/vk/descriptor_set_layout.cpp
    src/gl/impl/backend/vk/descriptor_set_layout.hpp
    src/gl/impl/backend/vk/descriptor.cpp
    src/gl/impl/backend/vk/descriptor.hpp
    src/gl/impl/backend/vk/descriptor.inl
    src/gl/impl/backend/vk/fence.cpp
    src/gl/impl/backend/vk/fence.hpp
    src/gl/impl/backend/vk/frame_admin.cpp
    src/gl/impl/backend/vk/frame_admin.hpp
    src/gl/impl/backend/vk/frame_admin.inl
    src/gl/impl/backend/vk/framebuffer.cpp
    src/gl/impl/backend/vk/framebuffer.hpp
    src/gl/impl/backend/vk/image_view.cpp
    src/gl/impl/backend/vk/image_view.hpp
    src/gl/impl/backend/vk/image.hpp
    src/gl/impl/backend/vk/image.cpp
    src/gl/impl/backend/vk/logical_device.cpp
    src/gl/impl/backend/vk/logical_device.hpp
    src/gl/impl/backend/vk/present.cpp
    src/gl/impl/backend/vk/present.hpp
    src/gl/impl/backend/vk/render_pass.cpp
    src/gl/impl/backend/vk/render_pass.hpp
    src/gl/impl/backend/vk/render_pass.inl
    src/gl/impl/backend/vk/sampler.cpp
    src/gl/impl/backend/vk/sampler.hpp
    src/gl/impl/backend/vk/semaphore.cpp
    src/gl/impl/backend/vk/semaphore.hpp
    src/gl/impl/backend/vk/submit.cpp
    src/gl/impl/backend/vk/submit.hpp
    src/gl/impl/backend/vk/swapchain.cpp
    src/gl/impl/backend/vk/swapchain.hpp
    src/gl/impl/backend/vk/tz_vulkan.cpp
    src/gl/impl/backend/vk/tz_vulkan.hpp
    src/gl/impl/backend/vk/vertex_binding_description.cpp
    src/gl/impl/backend/vk/vertex_binding_description.hpp
    src/gl/impl/backend/vk/vma.cpp

    # tz::gl (OpenGL Backend)
    src/gl/impl/backend/ogl/setup/opengl_instance.cpp
    src/gl/impl/backend/ogl/setup/opengl_instance.hpp
    src/gl/impl/backend/ogl/buffer.cpp
    src/gl/impl/backend/ogl/buffer.hpp
    src/gl/impl/backend/ogl/framebuffer.cpp
    src/gl/impl/backend/ogl/framebuffer.hpp
    src/gl/impl/backend/ogl/renderbuffer.cpp
    src/gl/impl/backend/ogl/renderbuffer.hpp
    src/gl/impl/backend/ogl/texture.cpp
    src/gl/impl/backend/ogl/texture.hpp
    src/gl/impl/backend/ogl/tz_opengl.hpp
    src/gl/impl/backend/ogl/tz_opengl.cpp
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
elseif(${TOPAZ_PROFILE})
    message(STATUS "BuildConfig: Profile")
    configure_profile(topaz)
else()
    message(STATUS "BuildConfig: Release")
    configure_release(topaz)
endif()

add_subdirectory(lib)
add_subdirectory(tools)
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
    target_compile_options(topaz PRIVATE -Wall -Wextra -Wdelete-non-virtual-dtor -pedantic-errors -fsanitize-undefined-trap-on-error)
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
target_link_libraries(topaz PUBLIC debugbreak glfw stbi)