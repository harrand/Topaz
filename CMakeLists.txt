cmake_minimum_required(VERSION 3.9)
project(topaz VERSION 0.6.0 DESCRIPTION "Cross-Platform Multimedia Engine")
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 17)
include_directories(topaz ${PROJECT_SOURCE_DIR}/inc)
include_directories(topaz ${PROJECT_SOURCE_DIR}/src)
link_directories(${PROJECT_SOURCE_DIR}/lib)
add_library(topaz SHARED
        src/audio/audio.cpp
        src/audio/audio.hpp
        src/audio/audio.inl

        src/graphics/glew.c
        src/graphics/graphics.cpp
        src/graphics/graphics.hpp
        src/graphics/graphics.inl
        src/graphics/mesh.cpp
        src/graphics/mesh.hpp
        src/graphics/shader.cpp
        src/graphics/shader.hpp
        src/graphics/shader.inl
        src/graphics/skybox.cpp
        src/graphics/skybox.hpp
        src/graphics/stb_image.h
        src/graphics/texture.cpp
        src/graphics/texture.hpp
        src/graphics/texture.inl

        src/physics/boundary.cpp
        src/physics/boundary.hpp
        src/physics/physics.cpp
        src/physics/physics.hpp

        src/camera.cpp
        src/camera.hpp
        src/asset.cpp
        src/asset.hpp
        src/gui.cpp
        src/gui.hpp
        src/gui.inl
        src/gui_display.cpp
        src/gui_display.hpp
        src/listener.cpp
        src/listener.hpp
        src/matrix.cpp
        src/matrix.hpp
        src/quaternion.cpp
        src/quaternion.hpp
        src/scene.cpp
        src/scene.hpp
        src/object.cpp
        src/object.hpp
        src/time.cpp
        src/time.hpp
        src/topaz.hpp
        src/transform.cpp
        src/transform.hpp
        src/utility.hpp
        src/utility.inl
        src/vector.hpp
        src/vector.inl
        src/window.cpp
        src/window.hpp
        src/window.inl src/physics/physics_object.cpp src/physics/physics_object.hpp)
target_link_libraries(topaz OpenGL32 SDL2 SDL2_mixer SDL2_ttf SDL2main assimp)