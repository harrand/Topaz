cmake_minimum_required(VERSION 3.9)
project(topaz VERSION 0.7.0 DESCRIPTION "Cross-Platform Multimedia Engine")
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 17)
# add_compile_options are global flags. They are used everywhere, not just in particular modules.
add_compile_options(-Wall -Wextra -pedantic-errors -O3)

add_library(topaz SHARED
        src/audio/audio.cpp
        src/audio/audio.hpp
        src/audio/audio.inl

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

        src/graphics/asset.cpp
        src/graphics/asset.hpp
        src/graphics/asset.inl
        src/graphics/camera.cpp
        src/graphics/camera.hpp
        src/graphics/frame_buffer.cpp
        src/graphics/frame_buffer.hpp
        src/graphics/frame_buffer.inl
        src/graphics/glew.c
        src/graphics/graphics.cpp
        src/graphics/graphics.hpp
        src/graphics/graphics.inl
        src/graphics/light.cpp
        src/graphics/light.hpp
        src/graphics/mesh.cpp
        src/graphics/mesh.hpp
        src/graphics/shader.cpp
        src/graphics/shader.hpp
        src/graphics/shader.inl
        src/graphics/skybox.cpp
        src/graphics/skybox.hpp
        src/graphics/static_object.cpp
        src/graphics/static_object.hpp
        src/graphics/stb_image.h
        src/graphics/texture.cpp
        src/graphics/texture.hpp
        src/graphics/texture.inl

        src/graphics/animated_texture.cpp
        src/graphics/animated_texture.hpp
        src/graphics/gui/button.cpp
        src/graphics/gui/button.hpp
        src/graphics/gui/display.cpp
        src/graphics/gui/display.hpp
        src/graphics/gui/gui.cpp
        src/graphics/gui/gui.hpp
        src/graphics/gui/gui.inl
        src/graphics/model.cpp
        src/graphics/model.hpp
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
        src/utility/numeric.cpp
        src/utility/numeric.hpp
        src/utility/numeric.inl
        src/utility/string.cpp
        src/utility/string.hpp
        src/utility/time.cpp
        src/utility/time.hpp
        src/utility/time.inl src/graphics/render_pass.cpp src/graphics/render_pass.hpp)
# here we express in terms of modules, not just build flags
target_include_directories(topaz PUBLIC ${PROJECT_SOURCE_DIR}/inc)
target_include_directories(topaz PRIVATE ${PROJECT_SOURCE_DIR}/src)

find_library(LIB_ASSIMP assimp PATHS ${PROJECT_SOURCE_DIR}/lib)
find_library(LIB_SDL2 SDL2 PATHS ${PROJECT_SOURCE_DIR}/lib)
find_library(LIB_SDL2_MIXER SDL2_mixer PATHS ${PROJECT_SOURCE_DIR}/lib)
find_library(LIB_SDL2_TTF SDL2_ttf PATHS ${PROJECT_SOURCE_DIR}/lib)
find_library(LIB_SDL2_MAIN SDL2main PATHS ${PROJECT_SOURCE_DIR}/lib)
find_library(LIB_OPENGL OpenGL32)

target_compile_definitions(topaz PUBLIC TOPAZ_DEBUG)
# public is for interface dependencies which we have, but no specific implementation dependencies that games wont need
target_link_libraries(topaz PUBLIC ${LIB_SDL2} ${LIB_SDL2_MIXER} ${LIB_SDL2_TTF} ${LIB_SDL2_MAIN} ${LIB_ASSIMP} OpenGL32)

#add_subdirectory(${PROJECT_SOURCE_DIR}/src/test ${PROJECT_BINARY_DIR})

add_library(unit_testing_utility src/test/lib/test_failure_exception.cpp)

#[[
file(GLOB UNIT_TEST_SOURCES RELATIVE ${PROJECT_SOURCE_DIR}/src *.cpp)
foreach(unit_test_source ${UNIT_TEST_SOURCES})
    # simply cut off the cpp
    string(REPLACE ".cpp" "" unit_test_name ${unit_test_source})
    message("found module: " ${unit_test_name})
    add_executable(${unit_test_name} ${unit_test_source})
    target_include_directories(${unit_test_name} PRIVATE ${PROJECT_SOURCE_DIR}/src)
    target_link_libraries(${unit_test_name} topaz unit_testing_utility)
endforeach()
]]#

# For the demo executable
add_executable(topaz_demo src/test.cpp)
target_include_directories(topaz_demo PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(topaz_demo topaz)