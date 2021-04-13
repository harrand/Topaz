cmake_minimum_required(VERSION 3.9)

project(TZVK)

add_executable(TZVK
    src/main.cpp)

if(${TOPAZ_VULKAN} AND ${TOPAZ_OGL})
    # Vulkan + OGL
    message(STATUS "RenderAPI: Vulkan + OpenGL (\"VKOGL\")")
    target_compile_definitions(TZVK PUBLIC -DTZ_VULKAN=1 -DTZ_OGL=1)
elseif(${TOPAZ_VULKAN})
    # Vulkan
    message(STATUS "RenderAPI: Vulkan (\"VK\")")
    target_compile_definitions(TZVK PUBLIC -DTZ_VULKAN=1 -DTZ_OGL=0)
endif()

if(${TOPAZ_DEBUG})
    message(STATUS "BuildConfig: Debug")
    target_compile_definitions(TZVK PUBLIC -DTZ_DEBUG=1)
else()
    message(STATUS "BuildConfig: Release")
    target_compile_definitions(TZVK PUBLIC -DTZ_DEBUG=0)
endif()

add_subdirectory(lib)

target_link_libraries(TZVK PUBLIC debugbreak glad glfw)