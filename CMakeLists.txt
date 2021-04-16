cmake_minimum_required(VERSION 3.9)
include(cmake/setup_configs.cmake)

file(READ ./version.txt TZ_VERSION)
# Reconfigure on changes to ./version.txt
configure_file(./version.txt topaz_version.xml)
set(CMAKE_CXX_STANDARD 20)

project(TZVK VERSION ${TZ_VERSION})

add_library(TZVK
    src/core/assert.hpp
    src/core/engine_info.cpp
    src/core/engine_info.hpp
    src/core/engine_info.inl
    src/core/game_info.hpp
    src/core/tz.cpp
    src/core/tz.inl
    src/core/tz.hpp

    src/gl/vk/common/basic_list.hpp
    src/gl/vk/common/polymorphic_list.hpp
    src/gl/vk/common/polymorphic_list.inl

    src/gl/vk/hardware/device_filter.cpp
    src/gl/vk/hardware/device_filter.hpp
    src/gl/vk/hardware/device_filter.inl
    src/gl/vk/hardware/device.cpp
    src/gl/vk/hardware/device.hpp
    src/gl/vk/hardware/queue_family.cpp
    src/gl/vk/hardware/queue_family.hpp

    src/gl/vk/setup/extension_list.cpp
    src/gl/vk/setup/extension_list.hpp
    src/gl/vk/setup/vulkan_instance.cpp
    src/gl/vk/setup/vulkan_instance.hpp

    src/gl/vk/validation/debug_callback.hpp
    src/gl/vk/validation/debug_messenger.cpp
    src/gl/vk/validation/debug_messenger.hpp
    src/gl/vk/validation/debug_messenger.inl
    src/gl/vk/validation/layer.cpp
    src/gl/vk/validation/layer.hpp
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
target_compile_definitions(TZVK PUBLIC -DTZ_VERSION=\"${TZ_VERSION}\")

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
target_link_libraries(TZVK PRIVATE glfw)
target_link_libraries(TZVK PUBLIC debugbreak)