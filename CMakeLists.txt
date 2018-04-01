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
        src/graphics/gui.cpp
        src/graphics/gui.hpp
        src/graphics/gui_display.cpp
        src/graphics/gui_display.hpp
        src/graphics/gui_widget.cpp
        src/graphics/gui_widget.hpp
        src/graphics/material.cpp
        src/graphics/material.hpp
        src/graphics/mesh.cpp
        src/graphics/mesh.hpp
        src/graphics/object.cpp
        src/graphics/object.hpp
        src/graphics/object.inl
        src/graphics/scene.cpp
        src/graphics/scene.hpp
        src/graphics/scene.inl
        src/graphics/shader.cpp
        src/graphics/shader.hpp
        src/graphics/shader.inl
        src/graphics/sprite.cpp
        src/graphics/sprite.hpp
        src/graphics/texture.cpp
        src/graphics/texture.hpp
        src/graphics/texture.inl

        src/physics/boundary.cpp
        src/physics/boundary.hpp
        src/physics/physics.cpp
        src/physics/physics.hpp

        src/camera.cpp
        src/camera.hpp
        src/command.cpp
        src/command.hpp
        src/command.inl
        src/data.cpp
        src/data.hpp
        src/engine.cpp
        src/engine.hpp
        src/engine.inl
        src/engine_meta.cpp
        src/engine_meta.hpp
        src/listener.cpp
        src/listener.hpp
        src/matrix.cpp
        src/matrix.hpp
        src/quaternion.cpp
        src/quaternion.hpp
        src/graphics/stb_image.h
        src/time.cpp
        src/time.hpp
        src/utility.hpp
        src/utility.inl
        src/vector.hpp
        src/vector.inl)
target_link_libraries(topaz OpenGL32 SDL2 SDL2_mixer SDL2_ttf SDL2main mdl)