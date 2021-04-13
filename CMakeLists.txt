cmake_minimum_required(VERSION 3.9)

file(READ ./version.txt TZ_VERSION)
set(CMAKE_CXX_STANDARD 20)

project(TZVK VERSION ${TZ_VERSION})

add_executable(TZVK
    src/core/tz.cpp
    src/core/tz.inl
    src/core/tz.hpp
    src/main.cpp)

message(STATUS "Topaz Vulkan v${TZ_VERSION}")

if(${TOPAZ_VULKAN} AND ${TOPAZ_OGL})
    # Vulkan + OGL
    message(STATUS "RenderAPI: Vulkan + OpenGL (\"VKOGL\")")
    target_compile_definitions(TZVK PUBLIC -DTZ_VULKAN=1 -DTZ_OGL=1)
elseif(${TOPAZ_VULKAN})
    # Vulkan
    message(STATUS "RenderAPI: Vulkan (\"VK\")")
    target_compile_definitions(TZVK PUBLIC -DTZ_VULKAN=1 -DTZ_OGL=0)
endif()
target_compile_definitions(TZVK PUBLIC -DTZ_VERSION=\"${TZ_VERSION}\")

if(${TOPAZ_DEBUG})
    message(STATUS "BuildConfig: Debug")
    target_compile_definitions(TZVK PUBLIC -DTZ_DEBUG=1)
    target_compile_options(TZVK PRIVATE -g)
else()
    message(STATUS "BuildConfig: Release")
    target_compile_definitions(TZVK PUBLIC -DTZ_DEBUG=0)
    target_compile_options(TZVK PRIVATE -O3)
endif()

add_subdirectory(lib)

target_include_directories(TZVK PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(TZVK PUBLIC debugbreak glad glfw)