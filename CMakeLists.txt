cmake_minimum_required(VERSION 3.9)
project(topaz VERSION 0.6.0 DESCRIPTION "Cross-Platform Multimedia Engine")
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 17)
include_directories(topaz ${PROJECT_SOURCE_DIR}/inc)
include_directories(topaz ${PROJECT_SOURCE_DIR}/src)
link_directories(${PROJECT_SOURCE_DIR}/lib)
add_library(topaz SHARED
        src/graphics/glew.c
        src/audio/audio.cpp
        src/audio/audio.inl
        src/physics/boundary.cpp
        src/camera.cpp
        src/command.cpp
        src/command.inl
        src/data.cpp
        src/engine.cpp
        src/entity.cpp
        src/graphics/graphics.cpp
        src/graphics/graphics.inl
        src/graphics/gui.cpp
        src/graphics/gui_display.cpp
        src/graphics/gui_widget.cpp
        src/listener.cpp
        src/matrix.cpp
        src/graphics/mesh.cpp
        src/graphics/object.cpp
        src/graphics/object.inl
        src/physics/physics.cpp
        src/quaternion.cpp
        src/graphics/scene.cpp
        src/graphics/scene.inl
        src/graphics/shader.cpp
        src/graphics/shader.inl
        src/graphics/texture.cpp
        src/graphics/texture.inl
        src/time.cpp
        src/utility.inl
        src/vector.inl
)
target_link_libraries(topaz OpenGL32 SDL2 SDL2_mixer SDL2_ttf SDL2main mdl)