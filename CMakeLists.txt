cmake_minimum_required(VERSION 3.9)
include(cmake/setup_configs.cmake)

file(READ ./version.txt TZ_VERSION)
set(CMAKE_CXX_STANDARD 20)

project(TZVK VERSION ${TZ_VERSION})

add_executable(TZVK
    src/core/tz.cpp
    src/core/tz.inl
    src/core/tz.hpp
    src/core/assert.hpp
    src/main.cpp)

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

target_include_directories(TZVK PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(TZVK PUBLIC debugbreak glfw)