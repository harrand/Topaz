cmake_minimum_required(VERSION 3.9)
project(topaz VERSION 0.7.0 DESCRIPTION "Cross-Platform Multimedia Engine")
set(CMAKE_CXX_STANDARD 17)
set(UNDERLYING_GRAPHICS_API "TOPAZ_OPENGL")
set(TOPAZ_PROFILING TRUE)
# Add_compile_options are global flags. They are used everywhere, not just in particular modules.
message("CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
set(COMPILE_OPTIONS -Wall -Wextra -pedantic-errors)

if(CMAKE_BUILD_TYPE MATCHES Release)
    message(STATUS "Compilation set on Release mode.")
    add_compile_options(-Wall -Wextra -pedantic-errors -O3)
elseif(CMAKE_BUILD_TYPE MATCHES Debug)
    message(STATUS "Compilation set to Debug mode.")
    add_compile_options(-Wall -Wextra -pedantic-errors -O0)
else()
    message(FATAL_ERROR "Neither 'Release' nor 'Debug' specified for CMAKE_BUILD_TYPE. You must choose one.")
endif()

# Fetch and build freetype so Topaz can link against it.
include(ExternalProject)
set(FREETYPE_BINARY_DIR ${PROJECT_SOURCE_DIR}/extern/freetype/src/freetype_project-build)
set(FREETYPE_ROOT_DIR ${PROJECT_SOURCE_DIR}/extern/freetype)
set(FREETYPE_INCLUDE_DIR ${FREETYPE_ROOT_DIR}/src/freetype_project/include)
link_directories(${FREETYPE_BINARY_DIR})
ExternalProject_Add(freetype_project
        GIT_REPOSITORY git://git.sv.nongnu.org/freetype/freetype2.git
        PREFIX ${FREETYPE_ROOT_DIR}
        BINARY_DIR ${FREETYPE_BINARY_DIR}
        INSTALL_COMMAND "")

# Build Topaz.
add_library(topaz SHARED
        src/ext/glew.c
        src/ext/stb_image.h

        src/audio/audio.cpp
        src/audio/audio.hpp
        src/audio/audio.inl

        src/core/assert.cpp
        src/core/assert.hpp
        src/core/listener.cpp
        src/core/listener.hpp
        src/core/scene.cpp
        src/core/scene.hpp
        src/core/scene.inl
        src/core/topaz.hpp
        src/core/window.cpp
        src/core/window.hpp
        src/core/window.inl
        src/data/vector.hpp
        src/data/vector.inl

        src/data/matrix.cpp
        src/data/matrix.hpp
        src/data/quaternion.cpp
        src/data/quaternion.hpp
        src/data/transform.cpp
        src/data/transform.hpp
        src/data/tree.hpp
        src/data/tree.inl

        src/graphics/asset.cpp
        src/graphics/asset.hpp
        src/graphics/asset.inl
        src/graphics/camera.cpp
        src/graphics/camera.hpp
        src/graphics/frame_buffer.cpp
        src/graphics/frame_buffer.hpp
        src/graphics/frame_buffer.inl
        src/graphics/graphics.cpp
        src/graphics/graphics.hpp
        src/graphics/graphics.inl
        src/graphics/light.cpp
        src/graphics/light.hpp
        src/graphics/mesh.cpp
        src/graphics/mesh.hpp
        src/graphics/render_pass.cpp
        src/graphics/render_pass.hpp
        src/graphics/shader.cpp
        src/graphics/shader.hpp
        src/graphics/skybox.cpp
        src/graphics/skybox.hpp
        src/graphics/static_object.cpp
        src/graphics/static_object.hpp
        src/graphics/texture.cpp
        src/graphics/texture.hpp
        src/graphics/texture.inl

        src/graphics/animated_texture.cpp
        src/graphics/animated_texture.hpp
        src/graphics/attribute_collection.cpp
        src/graphics/attribute_collection.hpp
        src/graphics/font.cpp
        src/graphics/font.hpp
        src/graphics/gui/button.cpp
        src/graphics/gui/button.hpp
        src/graphics/gui/display.cpp
        src/graphics/gui/display.hpp
        src/graphics/gui/gui.cpp
        src/graphics/gui/gui.hpp
        src/graphics/gui/gui.inl
        src/graphics/image.cpp
        src/graphics/image.hpp
        src/graphics/model.cpp
        src/graphics/model.hpp
        src/graphics/renderable.hpp
        src/graphics/renderable_buffer.hpp
        src/graphics/renderable_buffer.inl
        src/graphics/sprite.cpp
        src/graphics/sprite.hpp
        src/graphics/gui/textfield.cpp
        src/graphics/gui/textfield.hpp
        src/graphics/gui/widget.cpp
        src/graphics/gui/widget.hpp

        src/physics/boundary.cpp
        src/physics/boundary.hpp
        src/physics/boundary.inl
        src/physics/dynamic_object.cpp
        src/physics/dynamic_object.hpp
        src/physics/dynamic_sprite.cpp
        src/physics/dynamic_sprite.hpp
        src/physics/physics.cpp
        src/physics/physics.hpp
        src/physics/physics_object.cpp
        src/physics/physics_object.hpp

        src/platform_specific/generic_buffer.cpp
        src/platform_specific/generic_buffer.hpp
        src/platform_specific/generic_buffer.inl
        src/platform_specific/mesh_buffer.cpp
        src/platform_specific/mesh_buffer.hpp
        src/platform_specific/mesh_buffer.inl
        src/platform_specific/shader_program.cpp
        src/platform_specific/shader_program.hpp
        src/platform_specific/shader_program.inl
        src/platform_specific/shader_storage_buffer.cpp
        src/platform_specific/shader_storage_buffer.hpp
        src/platform_specific/uniform.cpp
        src/platform_specific/uniform.hpp
        src/platform_specific/uniform.inl
        src/platform_specific/uniform_buffer.cpp
        src/platform_specific/uniform_buffer.hpp
        src/platform_specific/vertex_buffer.cpp
        src/platform_specific/vertex_buffer.hpp
        src/platform_specific/vertex_buffer.inl

        src/utility/constants.hpp
        src/utility/file.cpp
        src/utility/file.hpp
        src/utility/functional.cpp
        src/utility/functional.hpp
        src/utility/functional.inl
        src/utility/generic.cpp
        src/utility/generic.hpp
        src/utility/generic.inl
        src/utility/geometry.cpp
        src/utility/geometry.hpp
        src/utility/log.hpp
        src/utility/memory.hpp
        src/utility/memory.cpp
        src/utility/memory.inl
        src/utility/numeric.cpp
        src/utility/numeric.hpp
        src/utility/numeric.inl
        src/utility/render.hpp
        src/utility/render.cpp
        src/utility/string.cpp
        src/utility/string.hpp
        src/utility/time.cpp
        src/utility/time.hpp
        src/utility/time.inl)
# We depend on freetype.
add_dependencies(topaz freetype_project)
# here we express in terms of modules, not just build flags
target_include_directories(topaz PUBLIC ${PROJECT_SOURCE_DIR}/inc ${FREETYPE_INCLUDE_DIR})
target_include_directories(topaz PRIVATE ${PROJECT_SOURCE_DIR}/src)

find_library(LIB_ASSIMP assimp PATHS ${PROJECT_SOURCE_DIR}/lib)
find_library(LIB_SDL2 SDL2 PATHS ${PROJECT_SOURCE_DIR}/lib)
find_library(LIB_SDL2_MIXER SDL2_mixer PATHS ${PROJECT_SOURCE_DIR}/lib)
find_library(LIB_SDL2_MAIN SDL2main PATHS ${PROJECT_SOURCE_DIR}/lib)
find_library(LIB_OPENGL OpenGL32)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    message("Enabled TOPAZ_DEBUG.")
    target_compile_definitions(topaz PUBLIC TOPAZ_DEBUG ${UNDERLYING_GRAPHICS_API})
else()
    message("Disabled TOPAZ_DEBUG")
    target_compile_definitions(topaz PUBLIC ${UNDERLYING_GRAPHICS_API})
endif()
# public is for interface dependencies which we have, but no specific implementation dependencies that games wont need
#message(FATAL_ERROR ${FREETYPE_BINARY_DIR})
target_link_libraries(topaz PUBLIC ${LIB_SDL2} ${LIB_SDL2_MIXER} ${LIB_SDL2_MAIN} ${LIB_ASSIMP} freetype OpenGL32)
add_custom_command(TARGET topaz POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/res/dep"
        $<TARGET_FILE_DIR:topaz>)

# For the demo executable
add_executable(topaz_demo src/test.cpp)
target_include_directories(topaz_demo PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(topaz_demo topaz)

# Make unit testing targets visible.
add_subdirectory(src/test)